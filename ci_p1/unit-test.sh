#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline

pwd
ls -al

BASE_DIR=${PWD}
# 
if [ -z "${CMAKE_BUILD_DIR}" ] ; then 
	BASE_DIR=/app
	CMAKE_BUILD_DIR=${BASE_DIR}/build
	UNITTEST_REPORT_DIR=$BASE_DIR/reports
else
	echo "CMAKE_BUILD_DIR preset to '$CMAKE_BUILD_DIR'"
fi
TEST_DIR=${CMAKE_BUILD_DIR}/build-tip/cpp
VENDOR_DIR=${BASE_DIR}/vendor

cd ${TEST_DIR}
ctest
cd ${BASE_DIR}

# generate coverage
mkdir -p ${UNITTEST_REPORT_DIR}
echo "Writing coverage reports in ${UNITTEST_REPORT_DIR}"
GCOV=gcov
set -x
GCOV="${GCOV}" gcovr -j --verbose \
    --exclude-unreachable-branches \
    --exclude-throw-branches \
    --object-directory="${TEST_DIR}" \
    --xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
    --html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
    --sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml \
    --filter "${CPP_COVERAGE_FILTER}" \
	--exclude-directories "${VENDOR_DIR}" \
    $(if [ -n "${CPP_COVERAGE_EXCLUDE}" ]; then echo --exclude="${CPP_COVERAGE_EXCLUDE}"; fi)
set +x
