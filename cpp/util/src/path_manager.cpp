#include "path_manager.h"

fs::path PathManager::AmendPath(const fs::path& input_path)
{
#ifdef __WIN64
	if (input_path.string().size() > 260)
	{
		fs::path temp_path(windows_prefix_);
		temp_path += input_path;
		return temp_path;
	}
	else
	{
		return fs::path(input_path);
	}
#elif defined __linux__
	return fs::path(input_path);
#endif
}

fs::path PathManager::AsPath()
{
	return AmendPath(path_);
}

PathManager& PathManager::operator = (const PathManager& c)
{
	path_ = c.path_;
	return *this;
}

std::string PathManager::AsString()
{
	return AmendPath(path_).string();
}

PathManager& PathManager::Join(const std::string& input_path)
{
	path_ /= fs::path(input_path);
	return *this;
}

PathManager& PathManager::Parent()
{
	path_ = path_.parent_path();
	return *this;
}

bool PathManager::IsDirectory()
{
	// Fix the current path.
	fs::path temp_path = AmendPath(path_);

	return fs::is_directory(temp_path);
}

bool PathManager::IsFile()
{
	fs::path temp_path = AmendPath(path_);

	return fs::is_regular_file(temp_path);
}

bool PathManager::CreateDirectory()
{
	fs::path temp_path = AmendPath(path_);

	return fs::create_directory(temp_path);
}