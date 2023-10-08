
#ifndef ARROW_STATIC
#error arrow_static must be defined in conjunction with parquet
#endif

#ifndef PARQUET_STATIC
#error parquet_static must be defined in conjunction with parquet
#endif

#ifndef CH10_PARSE_EXE_NAME
#error "ch10parse.cpp: CH10_PARSE_EXE_NAME must be defined"
#endif

#ifndef TRANSLATE_1553_EXE_NAME
#error "translate_tabular_1553.cpp: TRANSLATE_1553_EXE_NAME must be defined"
#endif

#ifndef TRANSLATE_429_EXE_NAME
#error "translate_tabular_arinc429.cpp: TRANSLATE_429_EXE_NAME must be defined"
#endif


#include "meta_main.h"

int main(int argc, char* argv[])  // GCOVR_EXCL_LINE
{
    return MetaMain(argc, argv);  // GCOVR_EXCL_LINE
}