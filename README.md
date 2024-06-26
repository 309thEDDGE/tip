# Table of Contents

- [TIP](#tip)
- [Manifesto](#manifesto)
- [CMake Build](#cmake-build)
- [Conda Environment Build](#conda-environment-build)
- [Code Convention](#code-convention)
- [Linting](#linting)
- [Usage](#usage)
- [Exit Codes](#exit-codes)
- [`conda-forge` Package Build](#conda-forge-package-build)

# TIP

TIP -- Translate, Ingest, Parse (not in that order)

Parse [IRIG 106 Chapter 10](https://www.trmc.osd.mil/wiki/download/attachments/168165620/chapter10.pdf?version=1&modificationDate=1654195854977&api=v2) and export to Parquet files as intermediate data stores. In a second stage, convert data in an intermediate data store to Engineering Units (EU, ft, m/s, etc.) given an input file which conveys Interface Control Document (ICD) data.

`tip parse` requires as input a single Chapter 10 (ch10) file and outputs several directories and standalone files which contain the data extracted from various ch10 sub-packets. Directories have the extension `.parquet` and contain Parquet files and often a `_metadata.yaml` file with packet-specific metadata and provenance data related to the ch10 input file. A directory with the `.parquet` extension can be read directly with `pandas.read_parquet` and other Parquet tools. Files with a leading underscore are ignored. 

 Parsed (intermediate) data are generated by `tip parse` and can be used to understand recorder or bus state as a function of time and conveniently pack these data for later use. Parsed data include four primary components, each associated with a specific ch10 sub-packet: 
- raw data payload
- absolute or day-of-year time stamps on each payload unit
- sub-packet specific metadata
- ch10- and recorder-level metadata, if present

[Telemetry Attributes Transfer Standard](https://www.trmc.osd.mil/wiki/download/attachments/168165620/Chapter9.pdf?version=2&modificationDate=1660061064347&api=v2) (TMATS) and ch10 Time Data Packet data are output as single files in ASCII and Parquet format, respectively. All other outputs are Parquet directories. TMATS data come as a single binary blob which can be interpreted as ASCII and do not include the other three components mentioned above. Time Data Packet data are interpreted internally during parsing to assign absolute or day-of-year time based on the Relative Time Counter (RTC) given in each ch10 packet header. The reason time data are also given explicitly is to convey additional configuration metadata which are not used at parse time and often have recorder or platform specific meaning. All output products, excluding logs, are given a base name composed of the input ch10 file name and label specific to the relevant packet type: `<ch10 file name>_<packet type label>`.

Raw payloads are converted into EU by a translator. ICDs are necessary to decipher raw payloads, therefore TIP translators require as inputs the parsed data and a document which expresses ICD data in a specific format. To distinguish between various ICD formats used by different platforms and the specific input required by TIP, we have contrived a document called the Data Translation Specification (DTS). A DTS file is a yaml file with a schema specific to the packet type to which it is associated. TIP currently has translators for two different ch10 packet types, MIL-STD-1553 Format 1 (`tip translate 1553`) and ARINC-429 Format 0 (`tip translate arinc429`). Each require an input DTS with a specific schema. See the help menus of each for more information. 

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
```shell
docker build -t tip -f ubuntu_dev.Dockerfile .
```
Use `ubuntu_exe.Dockerfile` to build a docker image in which TIP is installed. Note that this provides a small-ish container which contains only TIP runtime libraries in addition to Ubuntu base image libraries. `cmake`, `git`, and other development utilities are not present. 

Alternately, prepare your system manually: 
    
### Essentials
 ```shell
apt update && apt install build-essential ninja-build cmake git
```
### yaml-cpp

```shell
apt install libyaml-cpp-dev
```

### GoogleTest
```shell
apt install googletest libgtest-dev libgmock-dev
```

### libtins
Prerequisites:
```shell
git clone https://github.com/mfontanini/libtins.git
apt install libpcap-dev libssl-dev cmake
```
Build and install:
```shell
cd libtins && mkdir build && cd build
make install
```

### arrow-cpp
Note: CMake issues several warnings when looking for packages required by arrow-cpp due to the inclusion of custom `find_package` modules which are not required when building from the debian package, which specifies and downloads the correct dependencies. 
```shell
apt update
apt install -y -V ca-certificates lsb-release wget
wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb
apt update
apt install -y -V libarrow-dev # For C++
apt install -y -V libparquet-dev # For Apache Parquet C++
```

### spdlog
```shell
apt install libspdlog-dev
```

### Build TIP
```shell
git clone <tip_repo>
cd <tip_repo> && mkdir build && cd build
cmake .. -GNinja
ninja install 
```

## Offline Builds (not recommended)

* Dependencies will be provided by the developer in a directory specified by the configure-time flag `USE_DEPS_DIR`
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
* `USE_NEWARROW`: Force use of compile-time macro `NEWARROW`. This is generally configured automatically for native linux and conda builds and is required for new versions of arrow. Causes source code to utilize new-style arrow-cpp library calls, which is relevant for arrow >~0.17. 

# Conda Environment Build

Install [conda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/)
for your given operating system. After conda is installed,
create the development environment. If the environment name already
exists you will want to update the environment instead.

## Note for Windows Users

The `Visual Studio 2017 C++ x86/64 Build Tools`
or a collection of tools which it comprises are required to build native C++ using Conda packages. The best way to obtain this tool set (currently) is to download VS 2022 Community and
enable required components:

* C++/CLI support for v141 build tools (14.16)
* MSVC v141 - VS 2017 C++ x64/x86 build tools (v14.16)
* C++ CMake tools for Windows
* Windows 10 SDK (10.0.18362.0) (*Exact version may not be necessary.*)
 
Documentation in [conda-forge.org](https://conda-forge.org/docs/maintainer/knowledge_base.html#particularities-on-windows) indicates that this MSVC version is only appropriate for Python version 3.5-3.7. However, this version was confirmed to be compatible with Python 3.9.

In order to build with the proper tool chain and reference the correct runtime, environment variables must be set in the command prompt or powershell in which the Conda environment is activated (see below). The easiest way to configure environment variables for both the Conda development environment and Windows C++ tool chain is

1. Open the `x64 Native Tools Command Prompt for VS 2022` (or use the powershell version)
2. Execute the `activate.bat` script in `<(mini)conda/installation/dir>/condabin/`, then follow the remaining steps to create, activate the dev environment, and finally build the project

## Create Conda Environment

Ensure that `conda` is up to date prior to creating the development environment.
```shell
conda update -n base conda
```

All tip dependencies have been packaged for Windows and Linux. Creating and activating the environment as follows will install all build-time dependencies, with exception of the build tool chain in Windows.
```shell
conda env create -f dev_environment.yaml
# conda env update -f dev_environment.yaml
```

Activate the development environment. Sometimes, depending on how conda
is installed, it must be activated via `source activate ...`.

```shell
conda activate tip-dev
# source activate tip-dev
```

## Build in Conda Environment
Create a build directory and configure, then build the project using CMake.
```shell
mkdir build
cd build
```

### Configure
#### Linux 
```shell
cmake .. -GNinja -DCONDA_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX 
```
#### Windows (Command Prompt)
```shell
cmake .. -GNinja -DCONDA_PREFIX=%CONDA_PREFIX% -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=%CONDA_PREFIX%
```
#### Windows (Powershell)
```shell
cmake .. -GNinja -DCONDA_PREFIX="$Env:CONDA_PREFIX" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$Env:CONDA_PREFIX"
```

### Build
```shell
cmake --build . --target install
```

If you wish to check code coverage, change the
build option to `-DCMAKE_BUILD_TYPE=Profile`. This will enable the
compiler flags for code coverage. Run the tests within the build directory to generate code coverage information, then generate the coverage report
```shell
./cpp/tests/tests
gcovr -r ../. --exclude-unreachable-branches --exclude-throw-branches --html-details ./overall-coverage.html .
```

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
- **tip**: Entry point for the parser, translator, and utilities. `tip -h`
- **tests**: Google tests executable to run entire test suite. `tests -h`

# Exit Codes

TIP follows Linux `/usr/include/sysexits.h` as closely as possible. Because TIP is (currently) intended to be built in both Linux and Windows, the header is included explicitly (`cpp/common/include/sysexits.h`).

For all executables, zero is returned if proper arguments are passed, arguments can be parsed, files/directories can be opened and/or created, system calls are executed, and no other error occurs. This implies that help menus and version printing sub-commands always result in an exit code of zero. All other cases shall result in non-zero exit codes. 

Most internal functions return boolean. Only functions which can fail at points where the logic is relevant to one of the exit codes, not a general internal error (`EX_SOFTWARE`), and where the elucidation of which is reasonably useful, return exit codes. High-level functions, such as those which return values to main, intepret non exit code-returning function successes and failures as 0 (`EX_OK`) and 70 (`EX_SOFTWARE`), respectively. 

# `conda-forge` Package Build
## Environments
Setup the Windows and Linux environments and build TIP locally following the [Cmake Build](#cmake-build) or [Conda Environment Build](#conda-environment-build) patterns prior to attempting the `conda-forge` build. `conda-forge` builds have additional complexity, partially due to using the `conda-build` toolchain. Save yourself time and verify that non-`conda-forge` builds succeed first.  

The procedure described in [Conda Environment Build](#conda-environment-build) is a non-`conda` build which is useful for iterative development simply because the build dependencies can be obtained easily via a `conda` environment. It does not use `conda-build` nor does it use the `conda-forge` build recipe found in the `conda-forge` `tip-feedstock` [repo](https://github.com/conda-forge/tip-feedstock). 

The TIP `conda-forge` package build automatically creates the necessary environment with build-time dependencies, builds in that environment, creates a runtime environment, installs the built TIP package in that environment, and executes tests to confirm success. For Windows builds, the process does not acquire the compilation toolchain, thus the need for installation of build tools as described in [Notes for Windows Users](#note-for-windows-users).

In preparation to `conda-forge` build, install `conda` and activate the base environment. It may be useful to review [Conda Environment Build](#conda-environment-build) for `conda` installation and activation procedures. 

### `conda-forge` Build
Steps for building the `conda-forge` package:
1. Clone tip-feedstock repo: `git clone https://github.com/conda-forge/tip-feedstock.git`
2. Create a `conda` environment which includes `conda-build`: `conda create -n tippkg -c conda-forge conda-build`
3. Activate the environment: `conda activate tippkg`
4. (tip-feedstock root) Build TIP Conda package: `conda build -c conda-forge --error-overdepending --error-overlinking --override-channels -m .ci_support/<configuration yaml> ./recipe`

The `configuration yaml` specifies compatible libraries for different `libarrow` versions. It also prescribes other package versions to ensure compatibility. Explicit reference to configuration is a requirement as of a recent change to the recipe requirements for `cstdlib` declaration. Reference a configuration file which is compatible with your operating system.  

## Test `conda-forge` Packages
If the `conda-forge` build stage succeeds, text printed to the terminal will include

```
Source and build intermediates have been left in <output dir>.
```
Locate the directory within `<output_dir>` and notate the package hash. An example TIP Conda package name is `tip-2.0.1-h2bc3f7f_4.tar.bz2` which has the hash "h2bc3f7f". 

Create a new conda environment and install the TIP package from the local build dir
```shell
 conda create -n tipcforge -c <output_dir> -c conda-forge tip
```
The ordering of repeated `-c` flag to specify channel is important. Activate the new environment and use `conda list` to verify that the installed version of TIP has the same hash identified above and the fourth column in the list output shows local package origin. Execute `tests` in the active environment to confirm that the package is viable. 

Official TIP `conda-forge` packages which have been built and published by `tip-feedstock` pipelines can be tested in a similar way. Choose from one of the [available packages](https://anaconda.org/conda-forge/tip/files), create a new Conda environment and install the chosen package, activate the environment, confirm the version, and execute tests. In example, a package with the name `linux-64/tip-2.0.2-he2bf4a2_1.conda` (version 2.0.2, hash he2bf4a2, build 1) can be installed in a new Linux environment via the command
```shell
conda create -n tipcforge -c conda-forge tip=2.0.2=he2bf4a2_1
```

## `conda-forge` TIP Package Update Process

Refer to the [`tip-feedstock` README](https://github.com/conda-forge/tip-feedstock#updating-tip-feedstock) for additional information about updating the `conda-forge` recipe. The procedure for updating the build number is discussed there and summarized here:
* If the TIP [source code](https://github.com/309thEDDGE/tip) release version is updated in the recipe, reset the build number to 0
* If only the recipe changes or the TIP source is modified and the version remains the same, increment the build number. 

The motivation to update the recipe, `meta.yaml`, in `tip-feedstock/recipe` can be described by the scenarios
* A new version of TIP has been released and the change must be reflected in the published `conda-forge` package
* TIP has new dependencies or new executables which must be checked for compatibility with the `conda-forge` build process. Both will also require updates to the recipe.
* The recipe itself needs to be updated in some other way 
* An automated PR into `tip-feedstock` fails

These scenarios require updates that fall into two general categories: TIP and recipe updates, and recipe-only updates. The first two scenarios fall into the TIP and recipe updates category, in which changes are usually accompanied by a new release so the build process and iterative debugging steps will be very similar. The third scenario is comparatively simple because there will likely be no interplay between the TIP source and recipe. The fourth scenario may fall into either category.  

When prompted to `conda-forge` build in the following section, refer to the build steps in [`conda-forge` build](#conda-forge-build).

### TIP and Recipe Updates
There is an interplay between non-`conda-forge` compilation and the configuration in the `conda-forge` recipe because changes to dependencies or source code may need to be reflected in the recipe. Compilation iteration between non-`conda-forge` and `conda-forge` builds may be necessary to succeed at both. 

The `meta.yaml` in `tip-feedstock/recipe` references an archived version of a TIP release in the parameters `source.url` and `source.sha256`. To iterate on a local version of TIP for simplicity instead of a formal TIP release, comment the above parameters, insert `source.path`, and define it as the local absolute path to the TIP repo root dir. If you reference the same directory in which TIP was previously built using the standard build paradigm then a `build` directory will be present and cause a failure in the `conda-forge` build. Delete the `build` directory prior to building.

Note that the recipe has several checks that are executed after the package is built and automatically installed in a new environment as part of the Conda build process. Test commands are defined in `test.commands`. Update these commands if executable commands are changed or other features become available that should also be tested. The build will fail if one of the commands returns a non-zero exit code. 

The iterative build process is as follows
1. Update local TIP source
2. Build TIP following the non-`conda-forge` paradigm
3. Revise as necessary until compilation is succesful
4. Build TIP according to the `conda-forge` pattern using the *temporarily* modified `meta.yaml` in the `tip-feedstock` repo. (The insertion of `source.path` is temporary and the proper values for `source.url` and `source.sha256` should be inserted in place of `source.path` prior to pushing.)
5. If both builds succeed, release a new version of TIP and generate a release archive, update `meta.yaml` with the correct `source.url` and `source.sha256` and attempt a local `conda-forge` build which references the new release tarball
6. If the build fails, return to step 1 or modify the recipe and return to step 4, depending on the fail mode.

### Recipe-only Updates
Builds which involve only changes to the recipe should be relatively easy to iterate: modify `meta.yaml` and build according to [`conda-forge` build](#conda-forge-build). 

### Post-update Procedure
If the local `conda-forge` build has succeeded, [test the package](#test-conda-forge-packages) prior to proceeding. Ensure that the recipe refers to a formal TIP release, recipe parameters `source.url` and `source.sha256` have been restored and `source.path` has been removed.  Commit the recipe to a `tip-feedstock` PR branch and merge only after all pipelines have completed without failure.

### Updates Required due to `tip-feedstock` Automated Build Failure
If the purpose for building the TIP `conda-forge` package is to troubleshoot an automated PR in the `tip-feedstock` repo which was created by the `regro-cf-autotick-bot`, it can be helpful to attempt a build using the recipe in the `main` branch of `tip-feedstock` first. If you can't build the working version of the recipe, then it is unlikely that you can troubleshoot and fix a failing PR. 

There are two paths to fix a failed PR. Either push updates to the recipe to the bot's branch or fork the `tip-feedstock` repo and fix the issues, then request a new bot PR. Instructions in the bot's PR describe a way to trigger the bot to cancel the PR and create a new one based off updated `main`.

*Experience has shown that issues are more easily addressed via a fork.* Dependency updates in automated PRs occasionally result in conflicts or cause C++ build failures. Changes to the source or recipe can be made in anticipation of an impending bot PR. After forking, create a new Conda build environment and attempt to follow the iterative process in [TIP and Recipe Updates](#tip-and-recipe-updates). Once successful, create a PR for `main` and confirm that pipelines have completed without failure, then merge the PR and signal the bot to cancel and create a new PR. 

## Notes
### Windows Specific 
If the `conda-forge` build step fails with the message 

> .. but ['ucrt'] not in reqs/run, (i.e. it is overlinking) (likely) or a missing dependency (less likely)

the cause is likely due to a configuration red herring in the `meta.yaml` `requirements.build/host/run` sections. `tip-feedstock` pipelines pull a configuration yaml from another `conda-forge` repo which de-conflicts the aforementioned error. This file is located at `github.com/conda-forge/conda-forge-pinning-feedstock/recipe/conda_build_config.yaml`. If this is the only error, try disabling error on overlinking by using the `--no-error-overlinking` instead of `--error-overlinking`. The same may be true for `--error-overdepending`, in which case use `--no-error-overdepending`. Then if the build succeeds, try pushing to `tip-feedstock` to see if the pipelines succeed. `tip-feedstock` pipelines *are* the `conda-forge` pipelines for the TIP `conda-forge` package. They are the source of truth for a successful build. 

