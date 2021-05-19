# Distribution Statement
DISTRIBUTION STATEMENT E. Distribution authorized to DoD Components only for
the sole purpose of Test and Evaluation without any or charge to the US 
Government or integration into licensed services for the US Government 
(1/1/2021). Other requests shall be referred to (309th Software Engineering 
Group - Hill AFB). 

# Ch10 Parser Notes 

## Table of Contents
- [Markdown](#markdown)
- [CMake Build](#cmake-build)
- [Code Convention](#code-convention)
- [Configuration Files](#configuration-files)
- [Usage](#usage)
- [Build Notes](#build-notes)
- [Preprocessor Definitions](#preprocessor-definitions)
- [Changelog](#changelog)

## Markdown
Use `markdown_to_html.py` in the tip_scripts directory to convert md file to html. Requires markdown2.py to be in PYTHONPATH. markdown2 is approved for use in SWEG.

## CMake Build

### Setup

#### Source

* `cd` to TIP root dir (let's call it `tip/`)
* `mkdir build`

#### Dependencies

* Dependencies for developers will be provided via the `tip_deps.zip` file which contains a "deps" directory which shall be copied into `tip/`
* `tip/deps` contains the following subdirectories:
	- arrow\_library\_dependencies
	- yaml-cpp-yaml-cpp-0.6.0 (itself containing a subdirectory of the same name)
	- googletest-release-1.8.1libs
	- libirig106
	- libtins
	- npcap
	
#### CMake

* `cd tip/build`
* `cmake .. -GNinja -D<option>=<value>`
	- add `-GNinja` for faster build
* Default options: 
	- CONTAINER=OFF (see below)
* Options given with `-D<option>=<value>`, multiple options with `-D<option1>=<val1> -D<option2>=<val2> ...`:
	- Container mode (`-DCONTAINER=ON`) - use shared libs in Linux-standard lib directories, default OFF
		- linux standard builds outside of a container are in progress
	
* build according to platform (see "Platform-Specific" below)
* Executables are placed in `tip/bin`
* Libraries are placed in `tip/bin/lib`

### Platform-Specific

#### Windows

* Must install cmake in Windows
* In command prompt or VS x64 command prompt:
	- `cd tip/build`
	- `cmake .. -GNinja -D<option>=<value>`
	- build with one of these commands
		- `ninja install` (if -GNinja was included)
		- `msbuild.exe INSTALL.vcxproj /property:Configuration=Release`
			- must be in VS x64 Native Tools Command Prompt
			- VS x64 Native Tools Command Prompt can be installed with VS, developer tools module enabled. License not required. From the Start menu navigate to the Visual Studio 2019 folder and select the correct command prompt.		
		- `cmake --build . --config Release`

* INSTALL will automatically run ALL_BUILD
* gtest/gmock and yaml-cpp libraries are included in `tip_deps.zip`, but if a rebuild is necessary, note the following:  
	- gmock/gtest  
		- msbuild automatically uses the dynamically linked runtime libraries so gtest/gmock must be build with the `"-Dgtest_force_shared_crt=ON"` flag to link those libs with the dynamic runtime. Building on x64 may also need specified using `-DCMAKE_GENERATOR_PLATFORM=x64`.  
		- example  
			- `cmake .. -Dgtest_force_shared_crt=ON -DCMAKE_GENERATOR_PLATFORM=x64`
			- `cmake --build . --config Release`
	- yaml-cpp
		- If using older version of yaml-cpp, update exceptions.h and exceptions.cpp  
			 `#if defined(_MSC_VER) && _MSC_VER < 1900`  
    		 `  #define YAML_CPP_NOEXCEPT _NOEXCEPT`  
			 `#else`  
    		 `	#define YAML_CPP_NOEXCEPT noexcept`  
			 `#endif`  
		- CMakeLists.txt updates
			- option(`YAML_CPP_BUILD_TESTS` “Enable testing” OFF)
			- option(`BUILD_SHARED_LIBS` "Build Shared Libraries" OFF)
			- option(`MSVC_SHARED_RT` "MSVC: Build with shared runtime libs (/MD)" ON)
		- example
			- `cmake .. -DCMAKE_GENERATOR_PLATFORM=x64`
			- `cmake --build . --config Release`


## Code Convention
Attempt to follow the Google C++ Style Guide which can be found at google.github.io/styleguide/cppguide.html. 

## Configuration Files

Configuration files must be present in `tip/conf` which is relative to the binary via `../conf`. Default configuration files
are located in `tip/conf/default_conf` and shall not be edited. Initially the user shall copy relevant `\*.yaml` files into 
`tip/conf` from `tip/conf/default_conf` and edit them as necessary.
If the `parse_and_translate.py` script is used, relevant config files are automatically copied from `tip/conf/default_conf` to 
`tip/conf` on the first run. Detailed comments for each configuration parameter are provided in the configuration files.  

### Standard config files

* `translate_conf.yaml`
* `parse_conf.yaml`

## Usage

### Helper Script (preferred)

* Must be run in an environment in which the sys.path can be appended to by the script (i.e., not the base conda environment)
* numpy required
* Call `parse_and_translate.py` script in TIP root directory with -h flag for helps
* Example: `python <path/to/TIP/root>/parse_and_translate.py -h`

### Call Executables Directly

#### --Standard executables
Navigate to `tip/bin` to call executables.

**tip\_parse.exe**: Parse ch10 file into intermediate Parquet files with raw 1553 payload information. If the second command line argument is not specified, the output parquet path is the folder containing the ch10 provided in the first argument.  
 `tip_parse.exe [path to \*.ch10 file] [optional argument, output directory]`  

 **tip\_parse\_video.exe**: Only available when built with `-DVIDEO=ON`. Parse ch10 file into intermediate Parquet files with raw 1553 payload information and video transport stream data. If the second command line argument is not specified, the output parquet path is the folder containing the ch10 provided in the first argument.  
 `tip_parse_video.exe [path to \*.ch10 file] [optional argument, output directory]`  

 **parquet\_video\_extractor.exe**: Only available when built with `-DVIDEO=ON`. Extract video transport stream data from parquet files. Exports TS files to a folder `<ch10path>/<ch10name>_video_TS` next to `<ch10path>/<ch10name>_video.parquet`  
 `parquet_video_extractor.exe [path to <ch10path>/<ch10name>_video.parquet folder]`  

  **tip\_translate.exe**: Translate raw 1553 data from Parquet files to parquet tables of enginering units.  The translated output is placed in the same folder as the parsed Parquet data.  The second argument is a DTS file, which stands for "Data Translation Specification"; this file is based on the Comet Flat Files, which come from the ICD specifications, but the DTS file is in a machine readable format. There is a sample at Sample_DTS.yaml.
`tip_translate.exe [path to parsed 1553 data directory, output from *ch10parse.exe ('<ch10path>/<ch10name>_1553.parquet')] [path to DTS]`

## Build Notes

### Preprocessor Definitions
Flags control debug print statement output verbosity, toggle enable/disable parsing of Ch10 packet types, indicate specific 1553 message selection, and specify output file type. **Default definitions are already handled in CMakeLists.txt**.

* `__WIN64` - Compatibility mode for Windows 64-bit. This is the only mode implemented currently.
* `DEBUG` - Debug level for print statements. If defined, print statements will be turned on but only printed if the value assigned is greater than each print statements threshold. Recommend 1 for normal use. Values in the range 3 to 5 are only useful in true debug mode because the output is too verbose for regular program functionality.
* `COLLECT_STATS` - Collect and print 1553 message statistics upon parse completion. 
* `LOCALDB` - Write database locally. Must be used in conjunction with `PARQUET` to write Parquet files to the local disk. (currently, only local writing is implemented)
* `_CRT_SECURE_NO_WARNINGS` - Definition for VS/cl to reduce warnings.
* `XDAT` - 1553 message selection. If defined, permits configuration file selection of 1553 messages. `select_specific_msgs` must be specified in [translate_conf.yaml](conf/default_conf/translate_conf.yaml). See *Configuration Files* for more information. 
* `VIDEO_DATA` - If defined, parse and record video data packets.
* `PARQUET` - When coupled with `LOCALDB`, causes Parquet format files to be written locally.
* `ARROW_STATIC` and `PARQUET_STATIC` - Must be defined when `PARQUET` is used and linker is configured to link statically.
* `ETHERNET_DATA` - (*Not Implemented*) If defined, parse and record Ethernet data packets. Does not cause Ethernet data payloads to be translated into distinct messages with engineering units.
* `NEWARROW` - build using newer arrow implementation (mainly used for container builds).
* `LIBIRIG106` - implement libirig106 libraries.

## Changelog

v0.1 - First tag
* Minimal 1553 payload or translated data validation 
* 1553 data written to Parquet format if selected via [preproccesor flags](#preprocessor-definitions)
* No build system

v0.1.1
* Reverse comet lookup for 1553 message name matching. Intended to mimic DRA's apparent matching strategy.
* Add buffering capability to ParquetContext - reduce I/O load related to head movement
* Fix Parquet Translation bug that causes each thread to drop the last <10,000 translated rows
* Add improved auto-bus map vs TMATS bus map selection
* Fix corner cases for translation of most float types
* Fix Float32 1750 and Float16
* Add configuration parameters to `parquet_translation1553.conf` and parse.conf and A10bus1553.conf

v0.1.2
* Add RMCOMET preprocessor definition to remove parse-time comet query
* RMCOMET: include channel ID --> LRU address, TMATS metadata in parquet file
* RMCOMET: translate-time bus mapping and comet query

v1.0.0
* YAML configuration files used with RMCOMET preprocessor definition
* CMake build system
* Add unit tests for most code associated with RMCOMET refactor
* Parquet video output
* Executable for extraction of TS video files from Parquet output
* `parse_and_translate.py` script for automatic parse and translation
* Fix boolean and signed bits bugs related to translation routine
* Optimize compilation for faster execution
* Reorganize directory structure for compatibility with build system
* Humble beginnings of containerized TIP and a potential in-container RESTful API
* Update deliverable script to avoid conflicts with other common Python packages
* Capability to ingest YAML DTS

v1.0.1
* Rename scripts directory to tip_scripts
* Update parse_and_translate.py to import new module name

v1.0.2
* Remove direct dependence on COMET
* RMCOMET mode logic applied permanently
* Remove references to potentially sensitive information

