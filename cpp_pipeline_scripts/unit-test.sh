#!/usr/bin/env bash
# CMAKE_BUILD_DIR defined by pipeline
# UNITTEST_REPORT_DIR defined by pipeline


# exit when any command fails
set -e


# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' ERR


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
ldd ./tests
./tests
cd ${BASE_DIR}

echo ""
echo Test Coverage
echo ""
mkdir -p ${UNITTEST_REPORT_DIR}
echo "Writing coverage reports in ${UNITTEST_REPORT_DIR}"
GCOV=gcov
GCOV="${GCOV}" gcovr -j --verbose \
    --exclude-unreachable-branches \
    --exclude-throw-branches \
    --object-directory="${TEST_DIR}" \
    --xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
    --html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
    --sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml \
    --filter "${CPP_COVERAGE_FILTER}" \
    $(if [ -n "${CPP_COVERAGE_EXCLUDE}" ]; then echo --exclude="${CPP_COVERAGE_EXCLUDE}"; fi)

echo ""
echo "--------------------Parser validation---------------------"
echo ""
cd $BASE_DIR
if [ ! -d ./bin ] ; then mv build/bin . ; fi
python tip_scripts/e2e_validation/run_end_to_end_validator.py --video /test/truth /test/test /test/log

echo ""
echo "--------------------End-to-end results------------------"
echo ""
# get newest log file
LOG_FILE="$(ls -1t /test/log/* | head -1)"

if [[ -z "$LOG_FILE" ]] ; then
	echo "No log files were found."
	exit 1
fi

EXIT_CODE=0 # Start by assuming success
if grep "Total raw 1553 data: PASS" "$LOG_FILE" ; then
	echo "Parser validation succeeded"
else
	echo "Parser validation failed"
	EXIT_CODE=1
fi

if grep "Total translated 1553 data: PASS" "$LOG_FILE" ; then
	echo "Translator validation succeeded"
else
	echo "Translator validation failed"
	# Don't set EXIT_CODE; the pipeline doesn't expect translation to succeed at this point
fi

echo ""
echo "-------------- Check for Alkemist presence --------------"
ldd ./bin/pqcompare
readelf -x .txtrp ./bin/pqcompare | grep 0x -m3
ldd ./bin/bincompare
readelf -x .txtrp ./bin/bincompare | grep 0x -m3
ldd ./bin/tests
readelf -x .txtrp ./bin/tests | grep 0x -m3
ldd ./bin/tip_parse_video
readelf -x .txtrp ./bin/tip_parse_video | grep 0x -m3
ldd ./bin/tip_translate
readelf -x .txtrp ./bin/tip_translate | grep 0x -m3

exit $EXIT_CODE