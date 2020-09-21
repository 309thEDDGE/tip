#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline

pwd
ls -al

cd ${CMAKE_BUILD_DIR}
ctest
cd ..

# generate coverage
mkdir -p ${UNITTEST_REPORT_DIR}
GCOV=gcov
set -x
GCOV="${GCOV}" gcovr -j --verbose \
    --exclude-unreachable-branches \
    --exclude-throw-branches \
    --object-directory="${CMAKE_BUILD_DIR}" \
    --xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
    --html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
    --sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml \
    --filter "${CPP_COVERAGE_FILTER}" \
    $(if [ -n "${CPP_COVERAGE_EXCLUDE}" ]; then echo --exclude="${CPP_COVERAGE_EXCLUDE}"; fi)
set +x
