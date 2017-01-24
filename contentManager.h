#ifndef ContentLoader_h__
#define ContentLoader_h__

#include <string>
#include <unordered_map>

#include "content.h"
#include "logger.h"
#include "fileManager.h"

#include <string>
#include <thread>
#include <mutex>
#include <vector>

/**
* Loads and manages memory of stuff loaded from disc
*/
class ContentManager
{
public:
	ContentManager(const std::string& contentRootDirectory = "", bool watchDirectory = true);
	//Don't allow copying since it would require shared_ptr and whatnot (for the destructor)
	ContentManager(ContentManager&&) = delete; //C++11! Wooo!
	ContentManager& operator=(const ContentManager&) = delete;
	~ContentManager();

	/**
	* Loads content. If the content is already loaded it will return a pointer to that instance instad of loading it again from disc.
	*
	* Can also be used to create content from memory. To do so, inherit from ContentCreationParameters and make
	* sure to set uniqueID to something useful when instantiating. Also pass "" as a path when calling Load(). See example below
	*
	*
	* Example of loading from disc:\n
	* `SomeClass* someObject = ContentManager.Load<SomeClass>("someFolder/someFile.someExtensions")`
	* 
	* Example of creating from memory:\n
	* \code
	* TextureCreationParameters parameters;
	* parameters.uniqueID = "createdTexture";
	* parameters.data = ... //fill with image data
	*
	* Texture* texture = ContentManager.Load<Texture>("", &parameters); //Note path, it's ""
	*
	* //Gets a pointer to the created resource. texture and texture0 both point to the same resource
	* Texture* texture0 = ContentManager.Load<Texture>("createdTexture", nullptr); //Note path, it's the same as uniqueID when creating. Also note that no contentParameters are passed.
	* \endcode
	*
	* \see Unload
	*
	* \param path path to content, leave blank if creating content
	* \param contentParameters
	*
	* \returns a pointer to the loaded content
	*/
	template<typename T>
	T* Load(const std::string& path, ContentParameters* contentParameters = nullptr) // TODO: Make two funcions? Maybe three where one has ContentParameters&?
	{
        std::string actualPath = contentRootDirectory + "/" + path;

		Content* existingContent = ContentAlreadyLoaded(actualPath, contentParameters);
		if(existingContent != nullptr)
			return static_cast<T*>(existingContent);

		Content* newContent = new T;
		if(!Load(newContent, actualPath, contentParameters))
		{
			DiskContent* diskContent = dynamic_cast<DiskContent*>(newContent);
			if(diskContent != nullptr)
			{
				if(!diskContent->CreateDefaultContent(actualPath.c_str(), this))
				{
					Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + actualPath + "\", and no default content could be created");

					delete newContent;
					newContent = nullptr;
				}
				else
					AddToMap(actualPath, newContent);
			}
			else
			{
				ContentCreationParameters* creationParameters = dynamic_cast<ContentCreationParameters*>(contentParameters);

				if(creationParameters != nullptr)
					Logger::LogLine(LOG_TYPE::WARNING, "Couldn't create content with ID \"" + std::string(creationParameters->uniqueID) + "\"");
				else
					Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load content at \"" + actualPath + "\"");

				delete newContent;
				newContent = nullptr;
			}
		}

		return static_cast<T*>(newContent);
	}

	/**
	* Same as Load, but doesn't get added to the map, and doesn't need any ID
	*/
	template<typename T>
	T* LoadTemporary(const std::string& path, ContentParameters* contentParameters = nullptr)
	{
		Content* newContent = new T;
		if(!LoadTemporary(newContent, path, contentParameters))
		{
			DiskContent* diskContent = dynamic_cast<DiskContent*>(newContent);
			if(diskContent != nullptr)
			{
				if(!diskContent->CreateDefaultContent( path.c_str(), this))
				{
					Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load temporary content at \"" + path + "\", and no default content could be created");

					delete newContent;
					newContent = nullptr;
				}
			}
			else
			{
				Logger::LogLine(LOG_TYPE::WARNING, "Couldn't load temporary content at \"" + path + "\"");

				delete newContent;
				newContent = nullptr;
			}
		}

		return static_cast<T*>(newContent);
	}

	/**
	* Gets already loaded content.
	*
	* \returns nullptr if the content didn't exists, otherwise a pointer to the content
	*/
	template<typename T>
	T* GetLoadedContent(const std::string& path)
	{
		Content* ptr = Exists(path);

		if(ptr != nullptr)
			return static_cast<T*>(ptr);

		return nullptr;
	}

	/**
	* Unloads the content at \p path and recreates it with \p contentParameters
	*/
	template<typename T>
	T* ForceLoad(const std::string& path, ContentParameters* contentParameters = nullptr)
	{
		if(path != "")
		{
			Content* content = Exists(path);

			if(content != nullptr)
			{
				content->refCount = 1;
				Unload(content);
			}
		}
		else
		{
			auto parameters = dynamic_cast<ContentCreationParameters*> (contentParameters);

			if(parameters == nullptr)
			{
				Logger::LogLine(LOG_TYPE::WARNING, "Tried reloading content without path and without ContentCreationParameters");
				return nullptr;
			}

			if(parameters->uniqueID == nullptr
			   || parameters->uniqueID[0] == '\0')
			{
				Logger::LogLine(LOG_TYPE::WARNING, "Tried reloading content without path and without uniqueID");
				return nullptr;
			}

			Content* content = Exists(parameters->uniqueID);

			if(content != nullptr)
			{
				content->refCount = 1;
				Unload(content);
			}
		}

		return Load<T>(path, contentParameters);
	}

	/**
	* Unloads the content at \p path and recreates it with \p contentParameters
	* if the load succeeded
	*/
	template<typename T>
	T* TryForceLoad(const std::string& path, ContentParameters* contentParameters = nullptr)
	{
		Content* originalContent = nullptr;
		
		if(path != "")
			originalContent = Exists(path);
		else
		{
			ContentCreationParameters* creationParameters = dynamic_cast<ContentCreationParameters*>(contentParameters);

			if(contentParameters == nullptr)
			{
				Logger::LogLine(LOG_TYPE::WARNING, "Tried force loading with no path and no ContentCreationParameters");
				return nullptr;
			}

			originalContent = Exists(creationParameters->uniqueID);
		}

		if(originalContent != nullptr)
			contentMap.erase(originalContent->GetPath());

		T* newContent = Load<T>(path, contentParameters);

		if(newContent != nullptr)
		{
			if(originalContent != nullptr)
			{
				originalContent->Unload(this);
				delete originalContent;
			}

			return newContent;
		}
		else
		{
			AddToMap(path, originalContent);
			return static_cast<T*>(originalContent);
		}
	}

	/**
	* Unloads all content loaded by this manager. Automatically called when destructor is called
	*
	* \see Unload(const std::string&)
	* \see Unload(Content*)
	*/
	void Unload();

	/**
	* Unloads the content at the given path. If there are no references to the content it will be completely unloaded
	* and loaded from disc the next time it's loaded
	*
	* Example:\n
	* \code
	* Texture* texture0 = contentManager.Load<Texture>("textures/someTexture.dds"); //loads from disc
	* Texture* texture1 = contentManager.Load<Texture>("textures/someTexture.dds"); //doesn't load from disc
	*
	* contentManager.Unload(texture0); //texture1 still hold reference, so no actual unloading is done
	*
	* Texture* texture2 = contentManager.Load<Texture>("textures/someTexture.dds"); //doesn't load from disc
	*
	* contentManager.Unload(texture1); //texture2 still hold reference, so no actual unloading is done
	* contentManager.Unload(texture2); //no more references, so the content is actually unloaded here
	*
	* Texture* texture3 = contentManager.Load<Texture>("textures/someTexture.dds"); //loads from disc
	* \endcode
	* 
	* \see Unload(Content*)
	*
	* \param path path to content to unload
	*/
	void Unload(const std::string& path);

	/**
	* Same as Unload(const std::string&), but takes a Content*
	*
	* \see Unload(const std::string&)
	*
	* \param content content to unload
	*/
	void Unload(Content* content);

	void IncreaseRefCount(Content* content) const;

	/**
	* Gets a semi-unique ID, it's actually the total count of times content has been loaded.
	*
	* That means that two calls to this method will return the same ID if no loading is
	* performed between the calls
	*/
	uint64_t GetSemiUniqueID() const;

	int GetRAMUsage() const;
	int GetDynamicVRAMUsage() const;
	int GetStaticVRAMUsage() const;

	void GetRAMAllocators(std::vector<std::pair<const char*, int>>& outData) const;
	void GetDynamicVRAMAllocators(std::vector<std::pair<const char*, int>>& outData) const;
	void GetStaticVRAMAllocators(std::vector<std::pair<const char*, int>>* outData) const;

	bool HasContentToHotReload() const;
	void HotReload();
private:
	std::string contentRootDirectory;

	//Hot reloading
	bool watchDirectory;
#ifdef _WIN32
	std::vector<HANDLE> changeHandles;
#else
    int watchDescriptor;
#endif
	std::unique_ptr<std::thread> directoryWatchThread;

	FileManager fileManager;

	//Simply counts up whenever content is loaded
	uint64_t uniqueID;

	Content* Exists(const std::string& path);
	void AddToMap(const std::string& path, Content* content);

	std::unordered_map<std::string, Content*> contentMap; //Use map if there are memory issues
	std::unordered_map<std::string, DiskContent*> reloadMap; //Use map if there are memory issues
	std::mutex reloadMapMutex;
	bool contentToHotReload;

	/**
	* Watches for file changes and reloads content as needed
	*/
	void WatchForFileChanges(const std::string& path);
	/**
	* Ran in a separate thread, waits for file changes and reloads content as needed
	*/
	void WaitForFileChanges(
#ifndef _WIN32
			int fileDescriptor

#endif
	);

	Content* ContentAlreadyLoaded(const std::string path, ContentParameters* contentParameters);
	bool Load(Content* newContent, const std::string path, ContentParameters* contentParameters);
	bool LoadTemporary(Content* newContent, const std::string path, ContentParameters* contentParameters);
};
#endif // ContentLoader_h__
