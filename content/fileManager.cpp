#include "fileManager.h"

#include "../logger.h"

namespace fs = std::experimental::filesystem;

FileManager::FileManager()
	: fileCount(0)
	, folderCount(0)
{ 
}

void FileManager::Init(const std::string& rootDirectory)
{
	this->rootDirectory = rootDirectory;
	rootFolder.reset(new Folder("root", nullptr));

	for(const auto& entry : fs::recursive_directory_iterator(rootDirectory.c_str()))
	{
		if(fs::is_regular_file(entry))
		{
			AddFile(entry);
		}
	}
}


std::tuple < std::vector<std::experimental::filesystem::path>
	, std::vector<std::experimental::filesystem::path>
	, std::vector<std::experimental::filesystem::path>> FileManager::Rebuild()
{
	std::unique_ptr<Folder> oldRootFolder(rootFolder.release());

	rootFolder.reset(new Folder("root", nullptr));
	Init(rootDirectory);

	auto newFiles = GetAllFiles(*rootFolder.get());
	auto oldFiles = GetAllFiles(*oldRootFolder.get());

	std::vector<fs::path> addedFiles;
	std::vector<fs::path> modifiedFiles;
	std::vector<fs::path> removedFiles;

	for(const auto& pair : newFiles)
	{
		auto iter = oldFiles.find(pair.first);
		if(iter != oldFiles.end())
		{
			if(pair.second.lastWriteDate != iter->second.lastWriteDate
			   || pair.second.size != iter->second.size)
			{
				modifiedFiles.push_back(pair.second.path);
			}

			oldFiles.erase(pair.first);
		}
		else
			addedFiles.push_back(pair.second.path);
	}

	for(const auto& pair : oldFiles)
	{
		auto iter = newFiles.find(pair.first);
		if(iter == newFiles.end())
			removedFiles.push_back(pair.second.path);
	}

	Logger::LogLine(LOG_TYPE::DEBUG, "File system tree rebuilt with ", addedFiles.size(), " new files, ", modifiedFiles.size(), " modified files, and ", removedFiles.size(), " removed files");

	return std::make_tuple(addedFiles, modifiedFiles, removedFiles);
}

void FileManager::AddFile(const std::experimental::filesystem::directory_entry& file)
{
	Folder* folder = BrowseTo(file.path(), true);
	if(folder->Get(file.path().filename().string()).path != "")
		Logger::LogLine(LOG_TYPE::DEBUG, "Overwriting existing file at " + file.path().string());

	folder->AddFile(file);
}

bool FileManager::FileExists(const std::experimental::filesystem::directory_entry& file)
{
	Folder* folder = nullptr;

	try
	{
		folder = BrowseTo(file, false);
	}
	catch(std::invalid_argument&)
	{
		return false;
	}

	return folder->Get(file.path().filename().string()).path != "";
}

bool FileManager::FileIsModified(const std::experimental::filesystem::directory_entry& file)
{
	Folder* folder = nullptr;

	try
	{
		folder = BrowseTo(file, false);
	}
	catch(std::invalid_argument& ex)
	{
		Logger::LogLine(LOG_TYPE::DEBUG, "Tried browsing to non-existing folder \"" + std::string(ex.what()) + "\" when adding checking if \"" + file.path().string() + "\" was modified");
		return true;
	}

	auto foundFile = folder->Get(file.path().string());
	if(foundFile.path == "")
	{
		Logger::LogLine(LOG_TYPE::DEBUG, "Tried browsing to non-existing file " + file.path().string());
		return true;
	}

	return foundFile.lastWriteDate != fs::last_write_time(file)
		|| foundFile.size != fs::file_size(file);
}

int FileManager::GetFileCount() const
{
	return fileCount;
}

int FileManager::GetFolderCount() const
{
	return folderCount;
}


std::unordered_map<std::string, FileData> FileManager::GetAllFiles(const Folder& folder)
{
	std::unordered_map<std::string, FileData> returnFiles;

	GetAllFilesRec(folder, returnFiles);

	return returnFiles;
}

void FileManager::GetAllFilesRec(const Folder& folder, std::unordered_map<std::string, FileData>& files)
{
	for(const auto& pair : folder.files)
		files.insert(std::pair<std::string, FileData>(pair.second.path.string(), pair.second));

	for(const auto& pair : folder.folders)
		GetAllFilesRec(*pair.second.get(), files);
}

Folder* FileManager::BrowseTo(const std::experimental::filesystem::path& path, bool createMissingFolders)
{
	Folder* returnFolder = rootFolder.get();

	fs::path folderPath;

	if(fs::is_regular_file(path))
		folderPath = path.parent_path();
	else
		folderPath = path;

	for(const auto& folder : folderPath)
	{
		Folder* nextFolder = returnFolder->Open(folder.string());
		if(nextFolder == nullptr)
		{
			if(!createMissingFolders)
				throw std::invalid_argument("Tried browsing to non-existing folder \"" + folder.string() + "\" when browsing to " + path.string());

			Folder* newFolder = new Folder(folder.string(), returnFolder);

			returnFolder->folders.insert(std::make_pair(folder.string(), std::move(std::unique_ptr<Folder>(newFolder))));

			nextFolder = newFolder;

			++folderCount;
		}

		returnFolder = nextFolder;
	}

	return returnFolder;
}
