#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
	set_exit_on_error
	setup
    export MINICONDA3_PATH="/home/user/miniconda3"
    export CONDA_CHANNEL_DIR="/local-channel"
    export PATH="$MINICONDA3_PATH/bin:${PATH}"
    export ARTIFACT_DIR="${ARTIFACT_FOLDER}/build-metadata"
    # mkdir $ARTIFACT_DIR
    #if [[ ! -d ${CMAKE_BUILD_DIR} ]]; then mkdir $CMAKE_BUILD_DIR; fi 


    echo -n "Installing Miniconda"
    dnf install wget -y
    wget --progress=dot:giga \
         https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
         && bash Miniconda3-latest-Linux-x86_64.sh -b -p $MINICONDA3_PATH

    echo -n "Installing conda-build"
    conda install conda-build -y
    echo -n "Change directory to conda-build recipes"
    cd tip_scripts
    echo -n "Building tip"
    ./conda_build.sh

    # cp -r $CONDA_CHANNEL_DIR/ $CMAKE_BUILD_DIR/
    # ls $CONDA_CHANNEL_DIR
    # echo "cmake build dir: $CMAKE_BUILD_DIR"
    
    ls $CMAKE_BUILD_DIR
    ls /
    ls /$CONDA_CHANNEL_DIR
    ls /$CONDA_CHANNEL_DIR/linux-64
    cp $CONDA_CHANNEL_DIR/linux-64/*tip* $ARTIFACT_DIR/
    ls $ARTIFACT_DIR
}

if ! is_test ; then 
	main $@
fi
