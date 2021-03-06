#include "contentManager.h"

#include "content.h"

#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <algorithm>

#ifndef _WIN32
#include <sys/inotify.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#endif

ContentManager::ContentManager(const std::string& contentRootDirectory /*= ""*/, bool watchDirectory /*= true*/)
	: contentRootDirectory(contentRootDirectory.empty() ? std::experimental::filesystem::current_path().string() : contentRootDirectory)
	, watchDirectory(watchDirectory)
	, uniqueID(0)
	, contentToHotReload(false)
{
	//if(watchDirectory)
		//WatchForFileChanges(contentRootDirectory);
}

ContentManager::~ContentManager()
{
	if(directoryWatchThread != nullptr)
	{
		watchDirectory = false;
		directoryWatchThread->join();
	}

	Unload();
}

void ContentManager::ForceHotReload(DiskContent* content)
{
    std::lock_guard<std::mutex> vectorGuard(forcedHotReloadsMutex);

    forcedHotReloads.push_back(content);
}

void ContentManager::ForceHotReload(std::vector<DiskContent*> content)
{
    std::lock_guard<std::mutex> vectorGuard(forcedHotReloadsMutex);

    forcedHotReloads.insert(forcedHotReloads.end(), content.begin(), content.end());
}

void ContentManager::Unload()
{
	for(auto& iter : contentMap)
	{
		if(iter.second->IsLoaded())
		{
			iter.second->Unload(this);
		}
	}

	for(auto& iter : contentMap)
	{
		delete iter.second;
		iter.second = nullptr;
	}

	contentMap.clear();
}

void ContentManager::Unload(const std::string& path)
{
	auto iter = contentMap.find(path);
	if(iter->second == nullptr)
		return; //Already unloaded

	if(!iter->second->IsLoaded())
		return;

	if(iter == contentMap.end())
	{
		Logger::LogLine(LOG_TYPE::WARNING, "Trying to unload conten that has already been unloaded or hasn't been loaded at all");
		return;
	}

	iter->second->refCount--;
	if(iter->second->refCount > 0)
		return;

	iter->second->Unload(this);
	iter->second = nullptr;
	delete iter->second;

	contentMap.erase(iter->first);
}

void ContentManager::Unload(Content* content)
{
	if(content == nullptr)
		return;

	std::string path = content->path;

	if(path == "") //"Local" content, it's not in contentMap
	{
		delete content;
		return;
	}

	auto iter = contentMap.find(path);
	if(iter == contentMap.end())
		return;

	if(iter->second == nullptr)
		return; //Already unloaded

	iter->second->refCount--;
	if(iter->second->refCount > 0)
		return; //This content is used somewhere else

	iter->second->Unload(this);
	delete contentMap.at(iter->first);

	contentMap.erase(path);
}

void ContentManager::IncreaseRefCount(Content* content) const
{
	++content->refCount;
}

uint64_t ContentManager::GetSemiUniqueID() const
{
	return uniqueID;
}

int ContentManager::GetRAMUsage() const
{
	int ramUsage = 0;

	for(const auto& pair : contentMap)
		ramUsage += pair.second->GetRAMUsage();

	return ramUsage;
}

int ContentManager::GetDynamicVRAMUsage() const
{
	int vramUsage = 0;

	for(const auto& pair : contentMap)
		vramUsage += pair.second->GetDynamicVRAMUsage();

	return vramUsage;
}

int ContentManager::GetStaticVRAMUsage() const
{
	int vramUsage = 0;

	for(const auto& pair : contentMap)
		vramUsage += pair.second->GetStaticVRAMUsage();

	return vramUsage;
}

void ContentManager::GetRAMAllocators(std::vector<std::pair<const char*, int>>& outData) const
{
	for(const auto& pair : contentMap)
		if(pair.second->GetRAMUsage() > 0)
			outData.emplace_back(pair.first.c_str(), pair.second->GetRAMUsage());
}

void ContentManager::GetDynamicVRAMAllocators(std::vector<std::pair<const char*, int>>& outData) const
{
	for(const auto& pair : contentMap)
		if(pair.second->GetDynamicVRAMUsage() > 0)
			outData.emplace_back(pair.first.c_str(), pair.second->GetDynamicVRAMUsage());
}

void ContentManager::GetStaticVRAMAllocators(std::vector<std::pair<const char*, int>>* outData) const
{
	for(const auto& pair : contentMap)
		if(pair.second->GetStaticVRAMUsage() > 0)
			outData->emplace_back(pair.first.c_str(), pair.second->GetStaticVRAMUsage());
}

bool ContentManager::HasContentToHotReload() const
{
	return contentToHotReload;
}

void ContentManager::HotReload()
{
	if(reloadMap.empty())
		return;

	std::lock_guard<std::mutex> lock(reloadMapMutex);

	for(auto& pair : reloadMap)
	{
		// pair.second is always DiskContent*, so this is fine
		DiskContent* currentPointer = static_cast<DiskContent*>(contentMap[pair.first]);

        if(pair.second->ApplyHotReload())
        {
            currentPointer->Unload(this);
            currentPointer->Apply(pair.second);
        }
        else
            pair.second->Unload(this);

		delete pair.second;
	}

	reloadMap.clear();

	contentToHotReload = false;
}

Content* ContentManager::Exists(const std::string& path)
{
	auto iter = contentMap.find(path);

	if(iter != contentMap.end())
	{
		iter->second->refCount++;
		return iter->second;
	}

	return nullptr;
}

void ContentManager::AddToMap(const std::string& path, Content* content)
{
	std::string stringPath = path;

	std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

	contentMap.insert(std::pair<std::string, Content*>(stringPath, content));

	content->isLoaded = true;
	content->SetPath(stringPath.c_str());
}

void ContentManager::WatchForFileChanges(const std::string& path)
{
    bool success = true;
#ifdef _Win32
	TCHAR drive[4];
	TCHAR file[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];

	_tsplitpath_s(contentRootDirectory.c_str(), drive, 4, nullptr, 0, file, _MAX_FNAME, ext, _MAX_EXT);

	changeHandles.push_back(FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE));
	changeHandles.push_back(FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME));
	changeHandles.push_back(FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_DIR_NAME));

	for(const auto& handle : changeHandles)
	{
		if(handle == INVALID_HANDLE_VALUE)
		{
			Logger::LogLine(LOG_TYPE::WARNING, "Couldn't watch directory, INVALID_HANDLE_TYPE");
			success = false;
		}
	}
#else
    int fileDescriptor = inotify_init();
    watchDescriptor = -1;

    if(fileDescriptor == -1)
    {
        success = false;
        Logger::LogLine(LOG_TYPE::WARNING, "inotify_init failed");
    }
    else
        watchDescriptor = inotify_add_watch(fileDescriptor, contentRootDirectory.c_str(), IN_CLOSE_WRITE); // TODO: Error handling, remove watch
#endif

	if(success)
	{
		fileManager.Init(contentRootDirectory);
		directoryWatchThread.reset(new std::thread(&ContentManager::WaitForFileChanges, this
#ifndef _WIN32
        , fileDescriptor
#endif
        ));
	}

}

void ContentManager::WaitForFileChanges(
#ifndef _WIN32
        int fileDescriptor
#endif
)
{
    const static size_t POLL_TIMEOUT = 250;

#ifndef _WIN32
    const static auto BUFFER_LENGTH = (sizeof(inotify_event) + 16) * 1024;
    unsigned char buffer[BUFFER_LENGTH];

	struct pollfd pfd = { fileDescriptor, POLLIN, 0 };
#endif

	while(watchDirectory)
	{
#ifdef _WIN32
        bool update = true;

		DWORD waitStatus = WaitForMultipleObjects(static_cast<DWORD>(changeHandles.size()), &changeHandles[0], FALSE, 250);

		switch(waitStatus)
		{
			case WAIT_OBJECT_0:
				changeHandles[0] = FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
				break;
			case WAIT_OBJECT_0 + 1:
				changeHandles[1] = FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
				break;
			case WAIT_OBJECT_0 + 2:
				changeHandles[2] = FindFirstChangeNotification(contentRootDirectory.c_str(), TRUE, FILE_NOTIFY_CHANGE_DIR_NAME);
				break;
			case WAIT_TIMEOUT:
				update = false;
				break;
			default:
				break;
		}
#else
		bool update = false;

        int pollValue = poll(&pfd, 1, POLL_TIMEOUT);

        if(pollValue < 0)
            Logger::LogLine(LOG_TYPE::INFO, "pollValue < 0"); // TODO: Remove
        else if(pollValue > 0) // pollValue == 0 => timeout
        {
            ssize_t length = read(fileDescriptor, buffer, BUFFER_LENGTH);

            if(length < 0)
                Logger::LogLine(LOG_TYPE::INFO, "length < 0"); // TODO: Remove
            else
				update = true;
        }
#endif

        if(forcedHotReloads.size() != 0)
        {
            // Sleep in case of multiple updates
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            std::lock_guard<std::mutex> vectorGuard(forcedHotReloadsMutex);
            std::lock_guard<std::mutex> mapGuard(reloadMapMutex);

            for(DiskContent* content : forcedHotReloads)
            {
                DiskContent* reloadContent = content->CreateInstance();
                reloadContent->SetPath(content->GetPath());

                // TODO: ContentManager isn't thread safe, shouldn't send this!
                if(reloadContent->BeginHotReload(std::string(content->GetPath()).c_str(), this) == CONTENT_ERROR_CODES::NONE) // TODO: More error handling?
                    reloadMap.insert(std::make_pair(content->GetPath(), reloadContent));
            }

            contentToHotReload = true;

            forcedHotReloads.clear();
        }

		if(update)
		{
            // Sleep in case of multiple updates
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

			//<new, modified, removed>
			auto tuple = fileManager.Rebuild();

			std::lock_guard<std::mutex> guard(reloadMapMutex);

			for(const auto& filePath : std::get<1>(tuple))
			{
				std::string filePathString = filePath.string();

				std::replace(filePathString.begin(), filePathString.end(), '\\', '/');

				if(contentMap.count(filePathString) > 0)
				{
					Content* content = contentMap[filePathString];

					DiskContent* diskContent = dynamic_cast<DiskContent*>(content);
					if(diskContent != nullptr)
					{
                        // Make a copy of the content, load it from disk in this thread and then update the "real"
                        // version when HotReload is called
						DiskContent* reloadContent = diskContent->CreateInstance();
						reloadContent->SetPath(filePathString.c_str());

						if(reloadContent->BeginHotReload(filePathString.c_str(), this) == CONTENT_ERROR_CODES::NONE) // TODO: More error handling?
						    reloadMap.insert(std::make_pair(content->GetPath(), reloadContent));
					}
				}
			}

			if(!reloadMap.empty())
				contentToHotReload = true;

			// TODO: Do I really need to do this every time...?
			inotify_rm_watch(fileDescriptor, watchDescriptor);
			close(fileDescriptor);
			fileDescriptor = inotify_init();
			watchDescriptor = inotify_add_watch(fileDescriptor, contentRootDirectory.c_str(), IN_CLOSE_WRITE); // TODO: Error handling, remove watch
		}
	}

	inotify_rm_watch(fileDescriptor, watchDescriptor);
	close(fileDescriptor);
}

Content* ContentManager::ContentAlreadyLoaded(const std::string path, ContentParameters* contentParameters)
{
	if(!path.empty())
		return Exists(contentRootDirectory + "/" + path);
	else
	{
		//Check if content with the given unique ID already exists
		ContentCreationParameters* creationParameters = dynamic_cast<ContentCreationParameters*>(contentParameters);

		if(creationParameters != nullptr
		   && creationParameters->uniqueID != nullptr
		   && creationParameters->uniqueID[0] != '\0')
		{
			return Exists(creationParameters->uniqueID);
		}
	}

	return nullptr;
}

bool ContentManager::Load(Content* newContent, const std::string path, ContentParameters* contentParameters)
{
    std::string actualPath;
    if(!path.empty())
        actualPath = contentRootDirectory + "/" + path;

    newContent->SetPath(actualPath.c_str());
    CONTENT_ERROR_CODES errorCode = newContent->Load(actualPath.c_str(), this, contentParameters);

	if(errorCode == CONTENT_ERROR_CODES::NONE)
	{
		++uniqueID;
		++newContent->refCount;

		if(!path.empty())
			AddToMap(contentRootDirectory + "/" + path, newContent);
		else
		{
			ContentCreationParameters* creationParameters = dynamic_cast<ContentCreationParameters*>(contentParameters);

			if(creationParameters == nullptr)
			{
				Logger::LogLine(LOG_TYPE::FATAL, "Path is \"\" but couldn't cast contentParameters to ContentCreationParameters!");
				return false;
			}

			if(creationParameters->uniqueID == nullptr
			   || creationParameters->uniqueID[0] == '\0')
			{
				Logger::LogLine(LOG_TYPE::FATAL, "No uniqueID set for content without path!");
				return false;
			}

			AddToMap(creationParameters->uniqueID, newContent);
		}

		return true;
	}
	else
	{
		if(path.empty())
		{
			ContentCreationParameters* creationParameters = dynamic_cast<ContentCreationParameters*>(contentParameters);

			if(creationParameters != nullptr)
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't create content with ID \"" + std::string(creationParameters->uniqueID) + "\"");
			else
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't create content without path, and the ContentParameters couldn't be cast to ContentCreationParameters");
		}
		else
		{
			switch(errorCode)
			{
			case CONTENT_ERROR_CODES::UNKNOWN:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" due to an unknown error");
				break;
			case CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" because a file couldn't be opened");
				break;
			case CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" because a dependency file couldn't be opened");
				break;
			case CONTENT_ERROR_CODES::CONTENT_PARAMETER_CAST:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" because the content parameters failed a cast");
				break;
			case CONTENT_ERROR_CODES::CONTENT_CREATION_PARAMETERS_CAST:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" because the content creation parameters failed a cast");
				break;
			case CONTENT_ERROR_CODES::CREATE_FROM_MEMORY:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\" because the content couldn't create something from memory");
				break;
			default:
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + path + "\", but no CONTENT_ERROR_CODES was returned");
				break;
			}
		}

		return false;
	}
}

bool ContentManager::LoadTemporary(Content* newContent, const std::string path, ContentParameters* contentParameters)
{
    if(newContent->Load((contentRootDirectory + "/" + path).c_str(), this, contentParameters) == CONTENT_ERROR_CODES::NONE)
        return true;
    else
        return false;
}

bool ContentManager::HasLoaded(const std::string& path) const
{
    return contentMap.find(contentRootDirectory + "/" + path) != contentMap.end();
}

bool ContentManager::HasCreated(const std::string& path) const
{
    return contentMap.find(path) != contentMap.end();
}

const char* ContentManager::GetRootDir() const
{
	return contentRootDirectory.c_str();
}