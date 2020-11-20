#include "managed_path.h"

fs::path ManagedPath::AmendPath(fs::path input_path)
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

ManagedPath& ManagedPath::operator = (const ManagedPath& c)
{
	std::string temp_path = c.fs::path::string();
	this->assign(temp_path);
	return *this;
}

std::string ManagedPath::string()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
	return amended_path.fs::path::string();
}

std::string ManagedPath::RawString()
{
	return this->fs::path::string();
}

bool ManagedPath::is_regular_file()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
	return fs::is_regular_file(amended_path);
}

bool ManagedPath::is_directory()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
	return fs::is_directory(amended_path);
}

ManagedPath ManagedPath::parent_path()
{
	return ManagedPath(this->fs::path::parent_path());
}