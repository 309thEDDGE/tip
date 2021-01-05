// ch10parse.h

// Require logical/reasonable preprocessor definitions.

#ifndef PARQUET
#error PARQUET must be defined!
#endif

#ifdef XDAT
#ifndef LOCALDB
#error XDAT can only be defined in conjunction with LOCALDB
#endif
#endif

#ifdef PARQUET
#ifndef ARROW_STATIC
#error ARROW_STATIC must be defined in conjunction with PARQUET
#endif
#ifndef PARQUET_STATIC
#error PARQUET_STATIC must be defined in conjunction with PARQUET
#endif
#endif

#include "parser_helper_funcs.h"


int main(int argc, char* argv[])
{	
	
	if (argc < 2)
	{
		printf("Requires single argument, path to *.ch10 file\n");
		return 0;
	}

	ParserConfigParams config;
	std::string tip_root_path = "";
	if (!ValidateConfig(config, tip_root_path))
		return 0;

	ManagedPath input_path;
	ManagedPath output_path;
	char* arg2 = "";
	if (argc == 3)
		arg2 = argv[2];
	if (!ValidatePaths(argv[1], arg2, input_path, output_path))
		return 0;

	StartParse(input_path, output_path, config);
	return 0;
}

//extern "C"
//{
//	int RunParser(char* input_path, char* output_path, char* tip_path)
//	{
//		ParserConfigParams config;
//		if (!ValidateConfig(config, tip_path))
//			return 1;
//
//		ManagedPath mp_input_path;
//		ManagedPath mp_output_path;
//		if (!ValidatePaths(input_path, output_path, mp_input_path, mp_output_path))
//			return 1;
//
//		if(!StartParse(mp_input_path, mp_output_path, config))
//			return 1;
//
//		return 0;
//	}
//
//	/*int PyInit_tip_parse(char* input_path, char* output_path, char* tip_path) 
//	{
//		return RunParser(input_path, output_path, tip_path);
//	}*/
//
//	void PyInit_tip_parse() {}
//}