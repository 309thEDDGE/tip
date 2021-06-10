#!/usr/bin/env bash
## for CPP, sourceanalyzer is used to wrap the cmake command
## so we need sourceanalyzer AND all the build tools 
## IMPORTANT: use of -exclude switches seem to have NO effect on creating the scan model for this use case
# the pipeline will supply 
#  FORTIFY_ARTIFACT_DIR
#  FORTIFY_REPORT_FILE
#  CMAKE_BUILD_DIR
#  BUILD_TAG
#  FORTIFY_PROJECT_VERSION

# check for all necessary binaries
set -x
which sourceanalyzer
which fortifyclient
if [[ -z "${FORTIFY_ARTIFACT_DIR}" ]]; then FORTIFY_ARTIFACT_DIR="./artifacts"; mkdir -p "${FORTIFY_ARTIFACT_DIR}"; fi
# save working directory
WORKDIR="$(pwd)"
# prepare build directory
if [[ -z "${CMAKE_BUILD_DIR}" ]]; then CMAKE_BUILD_DIR="./build"; fi
# clear the build directory if it exists
if [[ -d "${CMAKE_BUILD_DIR}" ]]; then rm -rf ${CMAKE_BUILD_DIR}; fi
mkdir -p ${CMAKE_BUILD_DIR}
cd ${CMAKE_BUILD_DIR}

CMAKE="cmake"
CMAKE_MAKE_PROGRAM="make"
# be careful with parallelism level
MAKE_CMD="make -j1 all VERBOSE=0"

if [[ -z "${FORTIFY_PROJECT_VERSION}" ]]; then echo "Error! environment variable FORTIFY_PROJECT_VERSION undefined"; exit 1; fi
if [[ -z "${BUILD_TAG}" ]]; then echo "Error! environment variable BUILD_TAG undefined"; exit 1; fi

# clean fortify analysis model
sourceanalyzer -b ${BUILD_TAG} -clean
# accourdint to fortify documentation: "File specifiers do not apply to C, C++, or Objective-C++ languages."
# which pretty much makes the exclude list useless
# EXCLUDE_SWITCH_LIST is defined by the pipeline and consists of a set of -exclude switches. e.g '-exclude src/ -exclude include/'
CC="sourceanalyzer -b ${BUILD_TAG} gcc" \
    CXX="sourceanalyzer -b ${BUILD_TAG} g++" \
    AR="sourceanalyzer -b ${BUILD_TAG} ar" \
    ${CMAKE} -DBUILD_SSL=OFF -DBUILD_TESTS=ON -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} ..

${MAKE_CMD}

# go back to root folder
cd "${WORKDIR}"
# set report file if not already defined
if [[ -z "${FORTIFY_REPORT_FILE}" ]]; then FORTIFY_REPORT_FILE="${FORTIFY_ARTIFACT_DIR}/fortify_results.fpr"; fi
sourceanalyzer -b ${BUILD_TAG} -show-build-warnings | tee  "${FORTIFY_ARTIFACT_DIR}/show-build-warnings.txt"
sourceanalyzer -b ${BUILD_TAG} -show-files | tee  "${FORTIFY_ARTIFACT_DIR}/show-files.txt"
# SCAN_SWITCH_LIST is defined by the pipeline and consists of a set of -scan switches. e.g '-scan src/ -scan include/'
sourceanalyzer -b ${BUILD_TAG} -f "${FORTIFY_REPORT_FILE}" -scan
