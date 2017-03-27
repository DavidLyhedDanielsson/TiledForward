#ifndef folder_h__
#define folder_h__

#include <unordered_map>
#include <memory>
#include <experimental/filesystem>

#include "fileData.h"

class Folder
{
public:
	Folder(const std::string& name, Folder* parentFolder);
	~Folder() = default;

	std::string name;
	Folder* parentFolder;

	std::unordered_map<std::string, FileData> files;
	std::unordered_map<std::string, std::unique_ptr<Folder>> folders;

	Folder* Open(const std::string& name);
	FileData Get(const std::string& name);

	std::string GetPath() const;

	void AddFile(const std::experimental::filesystem::directory_entry& file);
};

inline bool operator==(const Folder& lhs, const Folder& rhs)
{
	return lhs.name == rhs.name;
}

inline bool operator!=(const Folder& lhs, const Folder& rhs)
{
	return !(lhs == rhs);
}

inline bool operator&&(const Folder& lhs, const Folder& rhs)
{
	for(const auto& pair : lhs.files)
	{
		if(rhs.files.count(pair.first) == 0)
			return false;
	}

	for(const auto& pair : lhs.folders)
	{
		if(rhs.folders.count(pair.first) == 0)
			return false;
	}

	return true;
}

#endif // folder_h__
