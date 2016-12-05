#include "folder.h"


Folder::Folder(const std::string& name, Folder* parentFolder)
	: name(name)
	, parentFolder(parentFolder)
{

}

Folder* Folder::Open(const std::string& name)
{
	auto iter = folders.find(name);
	if(iter == folders.end())
		return nullptr;

	return iter->second.get();
}

FileData Folder::Get(const std::string& name)
{
	auto iter = files.find(name);
	if(iter == files.end())
		return FileData();

	return iter->second;
}

std::string Folder::GetPath() const
{
	std::string returnString = name;

	if(parentFolder->name != "root")
		returnString = parentFolder->GetPath() + "/" + returnString;

	return returnString;
}

void Folder::AddFile(const std::experimental::filesystem::directory_entry& file)
{
	FileData newData;
	newData.path = file.path();
	newData.lastWriteDate = std::experimental::filesystem::last_write_time(file);
	newData.size = std::experimental::filesystem::file_size(file);

	files.insert(std::make_pair(file.path().filename().string(), newData));
}
