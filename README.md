# Table of Contents

- [TIP](#tip)
- [Manifesto](#manifesto)
- [CMake Build](#cmake-build)
- [Conda Build](#conda-build)
- [Code Convention](#code-convention)
- [Linting](#linting)
- [Usage](#usage)

# TIP

TIP -- Translate, Ingest, Parse (not in that order)

Parse [IRIG 106 Chapter 10](https://www.trmc.osd.mil/wiki/download/attachments/168165620/chapter10.pdf?version=1&modificationDate=1654195854977&api=v2) and export to Parquet files as intermediate data stores. In a second stage, convert data in an intermediate data store to Engineering Units (EU, ft, m/s, etc.) given an input file which conveys Interface Control Document (ICD) data.

`tip_parse` requires as input a single Chapter 10 (ch10) file and outputs several directories and standalone files which contain the data extracted from various ch10 sub-packets. Directories have the extension `.parquet` and contain Parquet files and often a `_metadata.yaml` file with packet-specific metadata and provenance data related to the ch10 input file. A directory with the `.parquet` extension can be read directly with `pandas.read_parquet` and other Parquet tools. Files with a leading underscore are ignored. 

 Parsed (intermediate) data are generated by `tip_parse` and can be used to understand recorder or bus state as a function of time and conveniently pack these data for later use. Parsed data include four primary components, each associated with a specific ch10 sub-packet: 
- raw data payload
- absolute or day-of-year time stamps on each payload unit
- sub-packet specific metadata
- ch10- and recorder-level metadata, if present

[Telemetry Attributes Transfer Standard](https://www.trmc.osd.mil/wiki/download/attachments/168165620/Chapter9.pdf?version=2&modificationDate=1660061064347&api=v2) (TMATS) and ch10 Time Data Packet data are output as single files in ASCII and Parquet format, respectively. All other outputs are Parquet directories. TMATS data come as a single binary blob which can be interpreted as ASCII and do not include the other three components mentioned above. Time Data Packet data are interpreted internally during parsing to assign absolute or day-of-year time based on the Relative Time Counter (RTC) given in each ch10 packet header. The reason time data are also given explicitly is to convey additional configuration metadata which are not used at parse time and often have recorder or platform specific meaning. All output products, excluding logs, are given a base name composed of the input ch10 file name and label specific to the relevant packet type: `<ch10 file name>_<packet type label>`.

Raw payloads are converted into EU by a translator. ICDs are necessary to decipher raw payloads, therefore TIP translators require as inputs the parsed data and a document which expresses ICD data in a specific format. To distinguish between various ICD formats used by different platforms and the specific input required by TIP, we have contrived a document called the Data Translation Specification (DTS). A DTS file is a yaml file with a schema specific to the packet type to which it is associated. TIP currently has translators for two different ch10 packet types, MIL-STD-1553 Format 1 (`tip_translate_1553`) and ARINC-429 Format 0 (`tip_translate_arinc429`). Each require an input DTS with a specific schema. See the help menus of each for more information. 

The output base name of translated data is similar to parsed data: `<ch10 file name>_<packet type label>_translated`. Translated data are output as directories which contain sub-directories and a `_metadata.yaml` file. Sub-directories are Parquet files and have names which are taken from the DTS. Each has a schema based on the decomposition of raw payload into EU as described by the DTS. The metadata file has provenance information and other data relevant to the associated packet type. 

Of the many sub-packets which the ch10 standard defines, many are not yet parsed by TIP and only two are translated. We welcome support from the community.

# Manifesto

Open source Chapter 10 parse and management tool

## Motivation

Engineers in academic labs or the private sector are routinely tasked with the interpretation of data from custom hardware/software or third-party test and measurement equipment. In typical fashion, a parse tool or suite of utilities, henceforth referred to as "parser", is curated by the team to accomplish this task and is one of the first tools introduced to new team members. It is an understatement to say that the parser is used regularly during the course of a project. Parser maintenance and enhancement is a collaborative effort that is valued because the existence of a common tool enables a coherent perspective of the data, minimizes individual dev and debug time, and encourages the creation of analysis tools that can be shared by all. Our hope is that TIP can become the common tool that breaks down the silos which operate on Chapter 10 data, or serve as an example that excites leaders to direct the creation of something better.  

## CLIs not GUIs

STEM-based engineers and data scientists are the likely consumers of data parsers and most of those folks are comfortable with CLIs. Command line tools are obviously easier to run in batch and incorporate into CI/pipelines. GUIs bloat the dependency tree, increase dev time, and greatly complicate integration testing. If you wish to make TIP a GUI application you can fork the project. 

## All of the Data

Most of the extant Chapter 10 parse tools are designed to extract a time series of a single data element or message component. This extraction pattern is sufficient for _x_ vs. time plots and only in the scenario that one wants to generate a few of them. To use 1553 bus data as an example, some platforms may record a dozen different 1553 busses, each with hundreds of messages and each of those with dozens or hundreds of constituent elements. Consider that this implies the possibility of tens of thousands of unique elements, and each element represents a time series. A single element only-capable parser/translator requires iterative execution to extract all the elements, and perhaps consumes the same raw data on each pass. 

The move to "big data" motivates the need to extract all of the data. This can be done efficiently in a single pass using threading. In the modern paradigm the concern of output data size on disk is a lower priority than rapid data access and can be mitigated by modern data formats. In a comparison of TIP against one common Chapter 10 GUI parser, TIP can parse and translate all data elements from all messages for each Chapter 10 sub-packet currently implemented faster than the legacy GUI can extract a single data element. 

## Modern Data Format

[Parquet](https://parquet.apache.org/) is the output format of choice for time series data. The majority of data which TIP currently parses are time series data which do not occur on a regular period or occur with sufficient jitter that a time tag is necessary at each period. Also the raw payloads which are time tagged in the Chapter 10 most often tag a message or word with multiple constituent elements. It makes sense to consider these data as tabular with one column to represent the time and others to handle element values. Parquet  is a columnar tabular format which includes automatic compression, so its memory and disk space usage are efficient. It can be read by many tools including pandas and Spark.  

[YAML](https://yaml.org/) is the output format for metadata and the input format for DTS files (ICDs converted to a format ready for TIP ingest) because it is human and machine readable. This is largely a decision based on preference over JSON. In practice, ICDs are formatted as pdf, csv or use propietary formats or exist in databases. The existence of a single DTS schema per bus/sub-packet type hints at the possiblity of a universal schema for all ICDs of a given bus. The standardization of ICDs in a machine-readable format would solve many problems for groups which rely on the Chapter 10 format. 

## Monkey See, Monkey Do: How to Contribute to TIP

TIP has two broad frameworks that can be leveraged to contribute to the code base. The first supports parsing of sub-packets from the Chapter 10. The second framework supports translation of parsed or intermediate data in Parquet tables, the output from the first stage, into Parquet files of engineering units with schema that mirror the structure of messages and elements in the DTS file. Both can be described as "monkey see, monkey do" in that it's best to follow an existing case and modify or extrapolate as necessary. 

A contribution to TIP in the form of additional sub-packet parsing capability entails implementation of the following: 
- concrete parser class for the new packet type 
- parquet writer class for the new type which uses an existing helper class via dependency injection
- custom metadata function(s) using the existing metadata framework
- several other minor modifications 

Contribution in the form a new translator is more complicated. The following list gives the broad strokes of required components:
- DTS yaml schema which conveys information required to identify and translate raw data payloads 
- ingest and data organization classes for the new DTS file
- message/word/datum lookup table which takes inputs from raw data payloads
- concrete translator class 
- custom metadata function(s)
- executable for the new translator using existing utils to validate input, etc.  
- CLI for the new executable using internal CLI library

One will be most successful in either of these efforts if initially assisted by someone who is familiar with TIP. Please reach out if you have questions. 

# CMake Build

## Ubuntu 22.04 LTS

Use `ubuntu_dev.Dockerfile` to build a development docker container image
```bash
docker build -t tip -f ubuntu_dev.Dockerfile .
```
Use `ubuntu_exe.Dockerfile` to build a docker image in which TIP is installed. Note that this provides a small-ish container which contains only TIP runtime libraries in addition to Ubuntu base image libraries. `cmake`, `git`, and other development utilities are not present. 

Alternately, prepare your system manually: 
    
### Essentials
 ```bash
apt update && apt install build-essential ninja-build cmake git
```
### yaml-cpp

```bash
apt install libyaml-cpp-dev
```

### GoogleTest
```bash
apt install googletest libgtest-dev libgmock-dev
```

### libtins
Prerequisites:
```bash
git clone https://github.com/mfontanini/libtins.git
apt install libpcap-dev libssl-dev cmake
```
Build and install:
```bash
cd libtins && mkdir build && cd build
cmake .. -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_CXX11=1 
make install
```

### arrow-cpp
Note: CMake issues several warnings when looking for packages required by arrow-cpp due to the inclusion of custom `find_package` modules which are not required when building from the debian package, which specifies and downloads the correct dependencies. 
```bash
apt update
apt install -y -V ca-certificates lsb-release wget
wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
apt update
apt install -y -V libarrow-dev # For C++
apt install -y -V libparquet-dev # For Apache Parquet C++
```

### spdlog
```bash
apt install libspdlog-dev
```

### Build TIP
```bash
git clone <tip_repo>
cd <tip_repo> && mkdir build && cd build

cmake .. -GNinja
ninja install 
```

# Conda Build

Install [conda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/)
for your given operating system. All tip dependencies have been
packaged for Windows and Linux. After conda is installed,
create the development environment. If the environment name already
exists you will want to update the environment instead.

Note for Windows users: The `Visual Studio 2017 C++ x86/64 Build Tools`
are required to build native C++ using Conda packages. The best way to
obtain this tool set (currently) is to download VS 2022 Community and
enable only that toolset, which includes required components `C++/CLI support for v141 build tools (14.16)` and `MSVC v141 - VS 2017 C++ x64/x86 build tools (v14.16)`. Documentation in [conda-forge.org](https://conda-forge.org/docs/maintainer/knowledge_base.html#particularities-on-windows) indicates that this MSVC version is only appropriate for Python version 3.5-3.7. However, this version was confirmed to be compatible with Python 3.9.

Create the environment
```shell
conda env create -f environment_local_build.yaml
# conda env update -f environment_local_build.yaml
```

Activate the development environment. Sometimes depending on how conda
is installed it must be activated via `source activate ...`.

```shell
conda activate tip-dev
# source activate tip-dev
```

Now create a build directory and configure, then build the project
```shell
mkdir build
cd build

# linux build
cmake .. -GNinja -DCONDA_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX 
cmake --build . --target install

# windows build (Command Prompt)
cmake .. -GNinja -DCONDA_PREFIX=%CONDA_PREFIX% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%CONDA_PREFIX%
cmake --build . --target install

# windows build (Powershell)
cmake .. -GNinja -DCONDA_PREFIX="$Env:CONDA_PREFIX" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$Env:CONDA_PREFIX"
cmake --build . --target install
```

If you wish to check code coverage, change the
build option to `-DCMAKE_BUILD_TYPE=Profile`. This will enable the
compiler flags for code coverage. Run the tests within the build directory to generate code coverage information, then generate the coverage report
```shell
./cpp/tests/tests
gcovr -r ../. --exclude-unreachable-branches --exclude-throw-branches --html-details ./overall-coverage.html .
```

## Offline Builds (not recommended)

* Dependencies will provided by the developer in a directory specified by the configure-time flag `USE_DEPS_DIR`
* `USE_DEPS_DIR` contains the following subdirectories:
	- arrow
	- yaml-cpp 
	- googletest
	- spdlog-1.8.2
	- tins
	- npcap (Windows only)
* Fine structure is determined from reading `cmake/CMake_linux_depsdir.txt|CMake_windows_depsdir.txt`
* If git isn't installed or if tip isn't tracked by git you can manually specify the version at configure time using `-DCI_COMMIT_TAG=<CI_COMMIT_TAG>` or `-DCI_COMMIT_SHORT_SHA=<CI_COMMIT_SHORT_SHA>` or if you are not using a git tag or hash use `-DUSER_VERSION=<user specified>`
* Executables are placed in `tip/bin`

## Build Options

See CMakeLists.txt for more details. Options are passed to CMake with the `-D<option>=<value>` pattern. Some options require values (an argument must be passed) and others can be used as switches (without arguments). 

* `USE_DEPS_DIR=<value>`: Specify a directory in which dependencies may be found. Used to indicate an offline build. Generally, all dependencies will be searched in the specified directory. See cmake/CMake_windows_depsdir.cmake or cmake/CMake_linux_depsdir.cmake for variables that must be configured to identify header and source locations and library names. 
* `USER_INSTALL_DIR=<value>`: Specify binary installation directory. Defaults confine installation artifacts to conda environments for conda builds, typical PATH locations for native linux and `<tip root directory>/bin` for offline builds.
* Compilation-time configuration of version:
  - Determined automatically from git if git is present and TIP source resides within a git repo. No user inputs required.
  - `CI_COMMIT_TAG=<value>`: Manually specify the commit tag when it can't be obtained automatically from git
  - `CI_COMMIT_SHORT_SHA=<value>`: Alternatively specify the commit short SHA when it can't be obtained automatically from git
  - `USER_VERSION=<value>`: Set the TIP display version if it can't be obtained by git or the tag/hash from git ought not be used
* `USE_NEWARROW`: Force use of compile-time macro `NEWARROW`. This is generally configured automically for native linux and conda builds and is required for new versions of arrow. Causes source code to utilize new-style arrow-cpp library calls, which is relevant for arrow >~0.17. 

# Code Convention
Attempt to follow the Google C++ Style Guide which can be found at google.github.io/styleguide/cppguide.html. All source code (`*.h`, `*.cpp`) in `tip/cpp` shall be formatted with `clang-format` from the root directory and configured by the `.clang-format` file. 

OS-specific usage:

- Linux:
  - Install `clang-format` via package manager
  - `clang-format -i ./cpp/**/**/*.h ./cpp/**/**/*.cpp`
- Windows:
  - Install LLVM toolchain for `clang-format.exe`
  - (PowerShell) `& 'C:\Program Files\LLVM\bin\clang-format.exe' -i (Get-ChildItem .\cpp\*\*\*.h) (Get-ChildItem .\cpp\*\*\*.cpp)`

# Linting

`cpplint` is the preferred linter and it is also used in the CI. `CPPLINT.cfg` files in the root dir and some sub-directories in `cpp/` specify the overall linting configuration and library-specific configuration, respectively. Only files in `tip/cpp` shall be linted. Pip install cpplint then use the following commands from the root dir:

- Linux: `cpplint ./cpp/**/**/*.h ./cpp/**/**/*.cpp`
- Windows (PowerShell): `cpplint (Get-ChildItem .\cpp\*\*\*.h) (Get-ChildItem .\cpp\*\*\*.cpp)`

# Usage 

Navigate to `tip/bin` to call executables or find them on the path if in a conda environment or TIP was built and installed.

Executables:

- **tests**: Google tests executable to run entire test suite. `tests -h`
- **tip\_parse**: Parse ch10 file into intermediate Parquet files. `tip_parse -h`  
- **parquet\_video\_extractor**: Extract video transport stream data from parquet files and export TS video files. `parquet_video_extractor -h`
- **tip\_translate_1553**: Translate raw 1553 data from Parquet files to parquet tables of enginering units. `tip_translate_1553 -h`
- **tip\_translate_arinc429**: Similar to `tip_translate_1553`. See CLI for usage: `tip_translate_arinc429 -h`	 
- **pqcompare**: Compare two parquet tables for equivalence. `pqcompare -h`
- **bincompare**: Compare two files at the byte level for equivalence. `bincompare -h`
- **validate\_yaml**: Validate a yaml file with an input schema. `validate_yaml -h`
