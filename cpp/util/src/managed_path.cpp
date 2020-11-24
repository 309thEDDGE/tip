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

ManagedPath& ManagedPath::operator /= (const ManagedPath& rhs)
{
	fs::path temp_path = this->append(rhs.fs::path::string());
	this->assign(temp_path.string());
	return *this;
}

ManagedPath ManagedPath::operator / (const ManagedPath& rhs)
{
	ManagedPath temp_path = *this;
	return temp_path /= rhs;
}

ManagedPath& ManagedPath::operator += (const ManagedPath& rhs)
{
	fs::path temp_path(this->fs::path::string());
	temp_path += fs::path(rhs.fs::path::string());
	this->assign(temp_path.string());
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

bool ManagedPath::CreateDir()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));

	// If the directory exists, return true.
	if (fs::is_directory(amended_path))
	{
		printf("ManagedPath::CreateDir(): Directory already exists - %s\n", 
			this->RawString().c_str());
		return true;
	}

	// The parent directory must exist.
	ManagedPath parent = this->parent_path();
	if (!parent.is_directory())
	{
		printf("ManagedPath::CreateDir(): Parent directory does not exist - %s\n",
			parent.RawString().c_str());
		return false;
	}

	// Multiple directory creation attempts for busy media and/or
	// file systems.
	for (int i = 0; i < max_create_dir_attempts_; i++)
	{
		// Create the directory using the amended path.
		if (fs::create_directory(amended_path))
		{
			break;
		}
		else
		{
			printf("ManagedPath::CreateDir(): Failed to created dir (attempt %d) - %s\n",
				i + 1, this->RawString().c_str());
		}

		// Sleep to give the OS some time before the next file is created.
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		// If the directory exists now, then break, otherwise try again.
		if (fs::is_directory(amended_path))
			break;
		else if(i == max_create_dir_attempts_ - 1)
		{
			printf("ManagedPath::CreateDir(): Failed to created dir after %d attemps - %s\n",
				max_create_dir_attempts_, this->RawString().c_str());
			return false;
		}
	}



	return true;
}