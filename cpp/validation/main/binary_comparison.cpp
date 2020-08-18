#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

bool OpenFile(const char* path, std::ifstream& ifs);
std::streamsize ReadBytes(std::ifstream& ifs, std::streamsize read_count, char* data);

int main(int argc, char* argv[])
{
	std::streamsize read_size = int(100e6);
	if (argc < 3)
	{
		printf("Requires two arguments, absolute path to two files to be compared.\n");
		return 0;
	}

	char* file1_path = argv[1];
	char* file2_path = argv[2];
	std::ifstream ifile1;
	std::ifstream ifile2;

	if (!OpenFile(file1_path, ifile1))
		return 0;
	if (!OpenFile(file2_path, ifile2))
		return 0;

	std::vector<char> file1_data(read_size);
	std::vector<char> file2_data(read_size);
	std::streamsize file1_read_count = 0;
	std::streamsize file2_read_count = 0;

	bool equal_result = false;
	int chunk_count = 0;
	while (true)
	{
		file1_read_count = ReadBytes(ifile1, read_size, file1_data.data());
		file2_read_count = ReadBytes(ifile2, read_size, file2_data.data());

		if (file1_read_count != file2_read_count)
		{
			printf("Read count not equal\n");
			printf("FAIL\n");
			break;
		}

		equal_result = std::equal(file1_data.begin(), file1_data.begin()+file1_read_count, file2_data.begin());
		if (!equal_result)
		{
			printf("Comparison of chunk %02d not equal\n", chunk_count);
			printf("FAIL\n");
			break;
		}

		if (file1_read_count != read_size)
		{
			printf("PASS\n");
			break;
		}

		chunk_count++;
	}

	ifile1.close();
	ifile2.close();
	return 0;
}

bool OpenFile(const char* path, std::ifstream& ifs)
{
	ifs.open(path, std::ios::binary);
	if (!(ifs.is_open()))
	{
		printf("Error opening file: %s\n", path);
		return false;
	}
	return true;
}

std::streamsize ReadBytes(std::ifstream& ifs, std::streamsize read_count, char* data)
{
	ifs.read(data, read_count);
	return ifs.gcount();
}