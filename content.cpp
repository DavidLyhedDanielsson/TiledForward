#include "content.h"

Content::Content()
	: refCount(0)
	, isLoaded(false)
{ }

bool Content::IsLoaded() const
{
	return isLoaded;
}

const char* Content::GetPath() const
{
	return path.c_str();
}

void Content::SetPath(const char* path)
{
	this->path = path;
}
