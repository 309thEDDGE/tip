#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
    set_exit_on_error
    setup
    export MINICONDA3_PATH="/home/user/miniconda3"
    export ARTIFACT_DIR="${ARTIFACT_FOLDER}/build-metadata/build-artifacts"
    export CONDA_CHANNEL_DIR="/local-channel"
    export PATH="$MINICONDA3_PATH/bin:${PATH}"
    export SCRIPT_START_DIR=$(pwd)

    echo "current working directory at start of script"
    pwd

    mkdir -p $ARTIFACT_DIR

    # echo "test" >> $ARTIFACT_DIR/test.txt

    echo -n "Installing Miniconda"
    dnf install wget -y
    wget --progress=dot:giga \
         https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
         && bash Miniconda3-latest-Linux-x86_64.sh -b -p $MINICONDA3_PATH

    # Creating build environment
    conda env create -f environment.yaml

    export "Running CMake"
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR
    cmake .. -GNinja -DCONDA_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_INSTALL_LIBDIR=lib

    echo -n "Installing conda-build"
    conda install conda-build -y
    echo -n "Change directory to conda-build recipes"
    cd tip_scripts
    echo -n "Building tip"
    ./conda_build.sh

    cd $CONDA_CHANNEL_DIR
    conda index -s linux-64 -s noarch

    cd $SCRIPT_START_DIR

    echo "tarballing files from local channel dir"
    tar -cvf local_channel.tar $CONDA_CHANNEL_DIR

    echo "copying tarball to artifact dir"
    cp local_channel.tar $ARTIFACT_DIR

    echo "show all contents of artifact dir"
    ls -hl $ARTIFACT_DIR
}

if ! is_test ; then
	main $@
fi
