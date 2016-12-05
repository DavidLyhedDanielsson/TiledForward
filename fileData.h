#ifndef fileData_h__
#define fileData_h__

#include <experimental/filesystem>

struct FileData
{
	FileData()
		: path()
		, lastWriteDate()
		, size(0)
	{ }

	std::experimental::filesystem::path path;
	std::experimental::filesystem::file_time_type lastWriteDate;
	std::uintmax_t size;

	bool operator==(const FileData& rhs) const
	{
		return path == rhs.path
			&& lastWriteDate == rhs.lastWriteDate
			&& size == rhs.size;
	}

	bool operator!=(const FileData& rhs) const
	{
		return !(*this == rhs);
	}
};

#endif // fileData_h__