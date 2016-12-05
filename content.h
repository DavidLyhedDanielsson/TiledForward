// Base class for any content that needs to be loaded from storage during runtime

#ifndef Content_h__
#define Content_h__

#include <string>
#include <typeinfo>


enum class CONTENT_ERROR_CODES
{
	UNKNOWN = 0
	, NONE
	, COULDNT_OPEN_FILE
	, CONTENT_PARAMETER_CAST
	, CONTENT_CREATION_PARAMETERS_CAST
	, CREATE_FROM_MEMORY
};

/**
* Used as a base class and passed to content when it's created
*
* \see ContentManager
*/
struct ContentParameters
{
	ContentParameters()
	{ };
	virtual ~ContentParameters()
	{ };
};

/**
* Used as a base class for when content should be created
*
* \see ContentManager
*/
struct ContentCreationParameters :
	public ContentParameters
{
	ContentCreationParameters(const char* uniqueID)
		: uniqueID(uniqueID)
	{ }
	virtual ~ContentCreationParameters()
	{ }

	/**
	* Used to identify content when creating it from memory
	*
	* \see ContentManager
	*/
	const char* uniqueID;
};

class ContentManager;

/**
* Interface for any content
*
* \see ContentManager
*/
class Content
{
    friend class ContentManager;
public:
	Content();
	/**
	* DO NOT USE FOR UNLOADING/REMOVING! USE UNLOAD INSTEAD!
	*/
	virtual ~Content() = default;

	virtual bool IsLoaded() const;

	virtual int GetStaticVRAMUsage() const = 0;
	virtual int GetDynamicVRAMUsage() const = 0;
	virtual int GetRAMUsage() const = 0;

	const char* GetPath() const;

protected:
	/**
	* Performs the actual loading
	*
	* \param path path to content to load
	* \param contentManager the content manager from which this is loaded. Useful if some Content contains other Content
	* \param contentParameters
	*
	* \returns whether or not loading succeeded
	*/
	virtual CONTENT_ERROR_CODES Load(const char* filePath, ContentManager* contentManager = nullptr, ContentParameters* contentParameters = nullptr) = 0;
	/**
	* Performs the actual unloading.
	*
	* \param contentManager
	*
	* \returns
	*/
    virtual void Unload(ContentManager* contentManager = nullptr) = 0;

	template<typename T>
	T* TryCastTo(ContentParameters* parameters)
	{
		T* ptr = dynamic_cast<T*>(parameters);
		//if(ptr == nullptr)
		//	Logger::LogLine(LOG_TYPE::WARNING, "Couldn't cast ContentParameters to " + std::string(typeid(T).name())); //GCC mangles, VC++ doesn't

		return ptr;
	}

	void SetPath(const char* path);

	int refCount; //If Unload is called and refCount == 0 it's safe to fully unload this content

private:
	std::string path; //TODO: Use something else for faster hashing/access?
	bool isLoaded;
};

/**
* Used as a base class for content which is created from memory
*
* Really just used as a divider since it contains no additional methods or members
*
* \see ContentManager
* \see DiskContent
*/
class MemoryContent :
	public Content
{
public:
	MemoryContent()
	{ }
	virtual ~MemoryContent()
	{ }
};

/**
* Used as a base class for content which is loaded from disk
*
* Needed to support hot reloading
*
* \see ContentManager for information regarding hot reloading
*/
class DiskContent :
	public Content
{
	friend class ContentManager;

public:
	DiskContent()
	{ }
	virtual ~DiskContent()
	{ }

	/**
	* When loading fails this method will be called and it should return
	* some usable default content
	*/
	virtual bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) = 0;
	virtual bool Apply(Content* content) = 0;

protected:
	virtual CONTENT_ERROR_CODES LoadTemporary(const char* filePath, ContentManager* contentManager = nullptr) = 0;

	/**
	* Used for hot reloading, should just return a new empty instance
	*/
	virtual DiskContent* CreateInstance() const = 0;

};

#endif // Content_h__
