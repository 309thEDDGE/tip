set GTEST_DIR=%SRC_DIR%
REM gmock headers have a warning that doesnt exist in Visual Studio 2008
REM and maintainers don't want to support older version
REM (https://github.com/google/googletest/issues/1848)
if NOT "%VS_YEAR%" == "2008" GOTO NO_EDIT
powershell -Command "(gc googlemock/include/gmock/gmock-matchers.h) -replace '4251 5046', '4251' | Out-File googlemock/include/gmock/gmock-matchers.h"

:NO_EDIT
REM Copy headers
xcopy /S %GTEST_DIR%\googletest\include %LIBRARY_INC%
xcopy /S %GTEST_DIR%\googlemock\include %LIBRARY_INC%
set GMOCK_DIR=googlemock

cd %GTEST_DIR%

REM Build and copy static libraries (Release)
mkdir build_static_mt
cd build_static_mt
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy lib\gtest.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
copy lib\gtest_main.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
copy lib\gmock.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
copy lib\gmock_main.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy static libraries (Debug)
mkdir build_static_mtd
cd build_static_mtd
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy lib\gtestd.lib %LIBRARY_LIB%\gtestd.lib
if errorlevel 1 exit 1
copy lib\gtest_maind.lib %LIBRARY_LIB%\gtest_maind.lib
if errorlevel 1 exit 1
copy lib\gmockd.lib %LIBRARY_LIB%\gmockd.lib
if errorlevel 1 exit 1
copy lib\gmock_maind.lib %LIBRARY_LIB%\gmock_maind.lib
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy static libraries with shared run-time library (Release)
mkdir build_static_md
cd build_static_md
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -D gtest_force_shared_crt=ON -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy lib\gtest.lib %LIBRARY_LIB%\gtest-md.lib
if errorlevel 1 exit 1
copy lib\gtest_main.lib %LIBRARY_LIB%\gtest_main-md.lib
if errorlevel 1 exit 1
copy lib\gmock.lib %LIBRARY_LIB%\gmock-md.lib
if errorlevel 1 exit 1
copy lib\gmock_main.lib %LIBRARY_LIB%\gmock_main-md.lib
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy static libraries with shared run-time library (Debug)
mkdir build_static_mdd
cd build_static_mdd
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -D gtest_force_shared_crt=ON -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy lib\gtestd.lib %LIBRARY_LIB%\gtest-mdd.lib
if errorlevel 1 exit 1
copy lib\gtest_maind.lib %LIBRARY_LIB%\gtest_main-mdd.lib
if errorlevel 1 exit 1
copy lib\gmockd.lib %LIBRARY_LIB%\gmock-mdd.lib
if errorlevel 1 exit 1
copy lib\gmock_maind.lib %LIBRARY_LIB%\gmock_main-mdd.lib
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy dynamic libraries (Release)
mkdir build_dynamic_mt
cd build_dynamic_mt
cmake -GNinja -DCMAKE_BUILD_TYPE=Relase -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -D gtest_build_tests=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy bin\gtest_dll.dll %LIBRARY_BIN%
if errorlevel 1 exit 1
copy lib\gtest_dll.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
copy bin\gmock.dll %LIBRARY_BIN%
if errorlevel 1 exit 1
copy lib\gmock.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy dynamic libraries (Debug)
mkdir build_dynamic_mtd
cd build_dynamic_mtd
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -D gtest_build_tests=ON -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy bin\gtest_dlld.dll %LIBRARY_BIN%\gtest_dlld.dll
if errorlevel 1 exit 1
copy lib\gtest_dlld.lib %LIBRARY_LIB%\gtest_dlld.lib
if errorlevel 1 exit 1
copy bin\gmockd.dll %LIBRARY_BIN%
if errorlevel 1 exit 1
copy lib\gmockd.lib %LIBRARY_LIB%
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy dynamic libraries with shared run-time library (Release)
mkdir build_dynamic_md
cd build_dynamic_md
cmake -GNinja -DCMAKE_BUILD_TYPE=Release -D gtest_force_shared_crt=ON -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -D gtest_build_tests=ON -DBUILD_SHARED_LIBS=ON  -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy bin\gtest_dll.dll %LIBRARY_BIN%\gtest_dll-md.dll
if errorlevel 1 exit 1
copy lib\gtest_dll.lib %LIBRARY_LIB%\gtest_dll-md.lib
if errorlevel 1 exit 1
copy bin\gmock.dll %LIBRARY_BIN%\gmock-md.dll
if errorlevel 1 exit 1
copy lib\gmock.lib %LIBRARY_LIB%\gmock-md.lib
if errorlevel 1 exit 1
cd %GTEST_DIR%

REM Build and copy dynamic libraries with shared run-time library (Debug)
mkdir build_dynamic_mdd
cd build_dynamic_mdd
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -D gtest_force_shared_crt=ON -D CMAKE_INSTALL_PREFIX=%LIBRARY_PREFIX% -D gtest_build_tests=ON -DBUILD_SHARED_LIBS=ON  -DCMAKE_PREFIX_PATH="%LIBRARY_PREFIX%" %GTEST_DIR%
ninja -j1
copy bin\gtest_dlld.dll %LIBRARY_BIN%\gtest_dll-mdd.dll
if errorlevel 1 exit 1
copy lib\gtest_dlld.lib %LIBRARY_LIB%\gtest_dll-mdd.lib
if errorlevel 1 exit 1
copy bin\gmockd.dll %LIBRARY_BIN%\gmockd-mdd.dll
if errorlevel 1 exit 1
copy lib\gmockd.lib %LIBRARY_LIB%\gmockd-mdd.lib
if errorlevel 1 exit 1
cd %GTEST_DIR%

