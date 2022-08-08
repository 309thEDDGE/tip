# Distribution Statement 
DISTRIBUTION STATEMENT E. Distribution authorized to DoD Components only for
the sole purpose of Test and Evaluation without any or charge to the US 
Government or integration into licensed services for the US Government 
(1/1/2021). Other requests shall be referred to (309th Software Engineering 
Group - Hill AFB).   

# Table of Contents

- [CMake Build](#cmake-build)
- [Code Convention](#code-convention)
- [Linting](#linting)
- [Singleuser Container](#singleuser-container)
- [Usage](#usage)

# CMake Build

## Ubuntu 22.04 LTS

Prepare system 
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

# Some warnings may emit due to arrow-cpp. Ignore. 
cmake .. -GNinja
ninja install 
```

## Conda

Install [conda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/)
for your given operating system. All tip dependencies have been
packaged for Windows and Linux. After conda is installed,
create the development environment. If the environment name already
exists you will want to update the environment instead.

Note for Windows users: The `Visual Studio 2017 C++ x86/64 Build Tools`
are required to build native C++ using Conda packages. The best way to
obtain this tool set (currently) is to download VS 2022 Community and
enable only that toolset. 

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

# Singleuser Container

The singleuser container is much more than a container in which the TIP executables reside. It is a full JupyterLab instance with an analytics environment and TIP executables on the PATH. The analytics environment includes numpy, matplotlib, pandas, and other common Python tools. 

Run the container
```bash
docker run -p 8888:8888 -v <host path>:<container path> registry.il2.dso.mil/skicamp/project-opal/tip:<latest>
```

Replace `<latest>` with the most recent container tag from https://code.il2.dso.mil/skicamp/project-opal/tip/container_registry/1904. Use flag `--rm` to delete the running container on exit. 

The easiest way to find the latest image is to get the commit hash from the most recent successful pipeline run on master and then search the container registry. `<host path>` is the path to the data you want to work with on your host system. (ch10, parquet, etc.). `<container path>` is where the data will be mounted within the container.

For example, `-v /home/user/data/ch10/:/data/` will mount the files in `/home/user/data/ch10/` on the host system to `/data/` in the container. If the container path does not exist, it will be created automatically.

To start the JuptyerLab GUI, copy+paste the `127.0.0.1:8888/lab?token=...` link provided at the end of the docker run command output into your browser.

## Using TIP Within the Singleuser Container

When running TIP or associated executables in the singleuser container, ensure that commands are executed from either

* the singleuser environment Jupyter notebook (and preceded with `!` to run shell commands)
* the JupyterLab shell and the singleuser conda environment is active 

# Usage 

Navigate to `tip/bin` to call executables or find them on the path if in a conda environment or TIP was built and installed.

Executables:

- **tests**: Google tests executable to run entire test suite
- **tip\_parse**: Parse ch10 file into intermediate Parquet files. `tip_parse -h`  
- **parquet\_video\_extractor** (useful CLI soon): Extract video transport stream data from parquet files. Exports TS files to a folder `<ch10path>/<ch10name>_video_TS` next to `<ch10path>/<ch10name>_video.parquet`  
`parquet_video_extractor [path to <ch10path>/<ch10name>_video.parquet folder]`  
- **tip\_translate_1553**: Translate raw 1553 data from Parquet files to parquet tables of enginering units. `tip_translate_1553 -h`
- **tip\_translate_arinc429**: Similar to `tip_translate_1553`. See CLI for usage: `tip_translate_arinc429 -h`	 
- **pqcompare** (useful CLI soon): Compare every cell of two parquet tables. Check for equivalent schema (column count, names, data type), then row by row. Used to quickly compare parquet files in end-to-end testing. 
- **bincompare** (useful CLI soon): Compare two files byte-for-byte. Used to quickly compare files in end-to-end testing. 
- **validate\_yaml** (useful CLI soon): Schema validate a yaml file given an input schema (also in yaml). Primarily used for debugging the schema validator code.

## Parse/translate Helper Script 

Generate Parquet files with engineering units with a single call. Removes the need for consecutive calls to `tip_parse` then `tip_translate_1553` or `tip_translate_arinc429`. This script is used by the end-to-end validation system. Due to simplified parse and translate CLIs, this script is nearly obsolete. It remains for the few users who rely on it as part of their workflow and will likely be removed when the end-to-end validation software is updated or rewritten. 

* Must be run in an environment in which the sys.path can be appended by the script (i.e., not a base conda environment)
* Call `parse_and_translate_cli.py` script in TIP root directory with -h flag for helps
* Example: `python <path/to/TIP/root>/parse_and_translate.py -h`

