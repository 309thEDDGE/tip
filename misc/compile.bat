cl /nologo /MD /W3 /Za /EHsc /DLOCALDB=1 /DDEBUG=3 /std:c++17 /Fech10parse.exe ^
/I..\include /I..\..\comet\include /IC:\Users\isaac.myers\Anaconda\anaconda3\Library\include ^
..\src\ch10parse.cpp ..\src\tmats.cpp ..\src\binbuff.cpp ..\src\ch10_tdf1.cpp ^
..\src\ch10_milstd1553f1.cpp ..\..\comet\src\parse_flat_msg.cpp ^
..\..\comet\src\parse_flat_wrd.cpp ..\..\comet\src\parse_flat_bit.cpp ^
..\..\comet\src\comet_data.cpp ^
..\src\ch10_packet_header.cpp ..\src\parse_worker.cpp ..\src\parse_manager.cpp ^
/link /LTCG /LIBPATH:C:\Users\isaac.myers\Anaconda\anaconda3\Library\lib zlib.lib libhdf5.lib libhdf5_cpp.lib

