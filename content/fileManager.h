#ifndef fileManager_h__
#define fileManager_h__

#include "folder.h"

#include "fileData.h"

#include <unordered_set>

#include <experimental/filesystem>

class Path;

class FileManager
{
public:
	FileManager();
	~FileManager() = default;

	FileManager& operator=(FileManager&& other) = default;

	void Init(const std::string& rootDirectory);
	/**
	* Rebuilds the tree from #rootDirectory and returns which files were added
	* , which files were modified and which files were removed
	*/
	std::tuple<std::vector<std::experimental::filesystem::path>
		, std::vector<std::experimental::filesystem::path>
		, std::vector<std::experimental::filesystem::path>> Rebuild();

	void AddFile(const std::experimental::filesystem::directory_entry& file);

	bool FileExists(const std::experimental::filesystem::directory_entry& file);
	bool FileIsModified(const std::experimental::filesystem::directory_entry& file);

	int GetFileCount() const;
	int GetFolderCount() const;

	//File* GetFile(const std::experimental::filesystem::directory_entry& file);
	//std::unordered_set<File*> GetAllFiles() const;
private:
	std::string rootDirectory;
	std::unique_ptr<Folder> rootFolder;

	int fileCount;
	int folderCount;

	//void AddFile(const FilePath& filePath, File* file);
	std::unordered_map<std::string, FileData> GetAllFiles(const Folder& folder);
	void GetAllFilesRec(const Folder& folder, std::unordered_map<std::string, FileData>& files);

	Folder* BrowseTo(const std::experimental::filesystem::path& path, bool createMissingFolders);
};

#endif // fileManager_h__
