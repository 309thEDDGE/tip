#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
    set_exit_on_error
    setup
    export ARTIFACT_DIR="${ARTIFACT_FOLDER}/build-metadata/build-artifacts"
    export CONDA_CHANNEL_DIR="${HOME}/local-channel"
    export SCRIPT_START_DIR=$(pwd)
    export CONDA_PREFIX=/opt/conda

    echo "current working directory at start of script"
    pwd

    mkdir -p $ARTIFACT_DIR

    # echo "test" >> $ARTIFACT_DIR/test.txt

    # =============================
    # Creating build environment
    # =============================
    conda env create -f environment.yaml --offline

    echo "Running CMake"
    mkdir -p $BUILD_DIR
    pushd $BUILD_DIR
    conda run -n tip-dev \
        cmake .. -GNinja \
        -DCONDA_PREFIX=$CONDA_PREFIX \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DCI_COMMIT_TAG=$CI_COMMIT_TAG \
        -DCI_COMMIT_SHORT_SHA=$CI_COMMIT_SHORT_SHA

    conda run -n tip-dev cmake --build . --target install
    conda run -n tip-dev ctest --rerun-failed --output-on-failure
    popd

    UNITTEST_REPORT_DIR=$BUILD_DIR/reports
    mkdir -p $UNITTEST_REPORT_DIR
    conda run -n tip-dev gcovr -j --verbose \
	      --exclude-unreachable-branches \
	      --exclude-throw-branches \
	      --object-directory="$BUILD_DIR/cpp" \
	      --xml ${UNITTEST_REPORT_DIR}/overall-coverage.xml \
	      --html ${UNITTEST_REPORT_DIR}/overall-coverage.html \
	      --sonarqube ${UNITTEST_REPORT_DIR}/overall-coverage-sonar.xml

    ls -la ${UNITTEST_REPORT_DIR}

    echo -n "Change directory to conda-build recipes"
    cd tip_scripts
    echo -n "Building tip"
    ./conda_build.sh

    cd $CONDA_CHANNEL_DIR
    conda index -s linux-64 -s noarch

    cd $SCRIPT_START_DIR

    echo "tarballing files from local channel dir"
    tar -cvf local_channel.tar $CONDA_CHANNEL_DIR

    # Raises error if tip channel tarball less than 1MB
    if (($(stat --printf="%s" local_channel.tar) < 1000000)); then
        echo "Tip channel tarball less than 1MB."
        echo "It is likely that the build failed silently. Exiting."
        exit 1
    fi

    echo "copying tarball to artifact dir"
    cp local_channel.tar $ARTIFACT_DIR

    echo "show all contents of artifact dir"
    ls -hl $ARTIFACT_DIR
}

if ! is_test ; then
	main $@
fi
