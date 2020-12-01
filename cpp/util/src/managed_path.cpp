#include "managed_path.h"

fs::path ManagedPath::AmendPath(fs::path input_path) const
{
#ifdef __WIN64
	if (input_path.string().size() > 259)
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

std::string ManagedPath::string() const
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
	return amended_path.fs::path::string();
}

std::string ManagedPath::RawString() const
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

bool ManagedPath::create_directory()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));

	// If the directory exists, return true.
	if (fs::is_directory(amended_path))
	{
		/*printf("ManagedPath::create_directory(): Directory already exists - %s\n", 
			this->RawString().c_str());*/
		return true;
	}

	// The parent directory must exist.
	ManagedPath parent = this->parent_path();
	if (!parent.is_directory())
	{
		printf("ManagedPath::create_directory(): Parent directory does not exist - %s\n",
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
			if (fs::is_directory(amended_path))
				break;
		}
		else
		{
			printf("ManagedPath::create_directory(): Failed to created dir (attempt %d) - %s\n",
				i + 1, this->RawString().c_str());
		}

		// Sleep to give the OS some time before the next file is created.
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		// If the directory exists now, then break, otherwise try again.
		if (fs::is_directory(amended_path))
			break;
		else if(i == max_create_dir_attempts_ - 1)
		{
			printf("ManagedPath::create_directory(): Failed to created dir after %d attemps - %s\n",
				max_create_dir_attempts_, this->RawString().c_str());
			return false;
		}
	}

	return true;
}

bool ManagedPath::remove()
{
	fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
	return fs::remove(amended_path);
}

ManagedPath ManagedPath::filename() const
{
	ManagedPath mp(this->fs::path::filename());
	return mp;
}

ManagedPath ManagedPath::stem() const
{
	ManagedPath mp(this->fs::path::stem());
	return mp;
}


ManagedPath ManagedPath::CreatePathObject(const ManagedPath& output_fname,
	const std::string& extension_replacement)
{
	if (extension_replacement == "")
	{
		// Want to keep current object unmodified. Create a copy.
		ManagedPath mp = *this;

		// Return the copy appended by the output file name.
		return mp /= output_fname.filename();
	}
	else
	{
		ManagedPath mp = *this;
		ManagedPath mp_file = output_fname.stem() += extension_replacement;
		return mp /= mp_file;
	}
}

void ManagedPath::GetFileSize(bool& success, uint64_t& result)
{
	success = false;
	result = 0;
	
	if (this->is_regular_file())
	{
		fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
		try
		{
			result = (uint64_t)fs::file_size(amended_path);
			success = true;
		}
		catch (fs::filesystem_error& e) 
		{
			printf("ManagedPath::GetFileSize Error: %s\n", e.what());
			success = false;
			result = 0;
		}
	}
}

void ManagedPath::GetListOfFiles(bool& success, std::vector<ManagedPath>& output_list,
	const std::vector<std::string>& exclude_matching)
{
	output_list.clear();

	if (this->is_directory())
	{
		std::string temp_path_str = "";
		ManagedPath temp_path;
		bool skip_entry = false;
		std::vector<ManagedPath> temp_output_list;
		std::vector<std::string> filenames_list;
		std::vector<std::string>::const_iterator it;
		fs::path amended_path = AmendPath(fs::path(this->fs::path::string()));
		for (auto& p : std::filesystem::directory_iterator(amended_path))
		{
			temp_path = ManagedPath(p.path());
			temp_path_str = temp_path.RawString();
			skip_entry = false;
			if (!temp_path.is_directory())
			{
				// If the current entry contains as a sub-string any of the
				// strings int the exclude_matching vector, do not include 
				// it in output_list.
				for (it = exclude_matching.cbegin(); it != exclude_matching.cend(); ++it)
				{
					if (temp_path_str.find(*it) != std::string::npos)
					{
						skip_entry = true;
						break;
					}
				}

				if (skip_entry)
					continue;

				temp_output_list.push_back(temp_path);
				filenames_list.push_back(temp_path.filename().RawString());
			}
		}

		// Get the vector of indices that sorts the filenames vector.
		IterableTools iter_tools;
		std::vector<size_t> sorted_inds = iter_tools.ArgSortAscending(filenames_list);

		// Fill the output_list with the objects, sorted by the file names.
		output_list.resize(temp_output_list.size());
		for (int i = 0; i < sorted_inds.size(); i++)
			output_list[i] = temp_output_list[sorted_inds[i]];

		success = true;
	}
	else
	{
		printf("ManagedPath::GetListOfFiles(): Object represents path that does not exist or "
			"is not a directory (%s)\n", this->RawString().c_str());
		success = false;
	}
}