#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline

pwd
ls -al

BASE_DIR=${PWD}
if [ -z "${CMAKE_BUILD_DIR}" ] ; then 
	# We are not in the pipeline; set vars for running locally
	BASE_DIR=/app
	CMAKE_BUILD_DIR=${BASE_DIR}/build
	UNITTEST_REPORT_DIR=$BASE_DIR/reports
fi
TEST_DIR=${CMAKE_BUILD_DIR}/cpp

echo ""
echo Unit Tests
echo ""
# For now, run cpp/tests because it is much faster than ctest
# In the future we might have to run ctest in order to get coverage statistics
# If we do, try to make our tests compatible with the --parallel option of ctest
cd ${TEST_DIR}
./tests
cd ${BASE_DIR}

echo ""
echo Test Coverage
echo ""
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
    $(if [ -n "${CPP_COVERAGE_EXCLUDE}" ]; then echo --exclude="${CPP_COVERAGE_EXCLUDE}"; fi)
set +x

echo ""
echo Parser validation
echo ""
set -x # Echo all commands
cd $BASE_DIR
mv build/bin .
python tip_scripts/pqpqvalidation/end_to_end_validator.py --video /test/truth /test/test /test/log
LOG_FILE="$(ls -1t /test/log/* | head -1)"
set +x

if [[ -z "$LOG_FILE" ]] ; then
	echo "Check parsing command; no log files were found."
	exit 1
elif grep "All validation set result: PASS" "$LOG_FILE" ; then
	echo "Parser validation succeeded"
	exit 0
else
	echo "Parser validation failed"
	exit 1
fi
