#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
    set_exit_on_error
    setup
    export MINICONDA3_PATH="/home/user/miniconda3"
    export ARTIFACT_DIR="${ARTIFACT_FOLDER}/build-metadata/build-artifacts"
    export CONDA_CHANNEL_DIR=$ARTIFACT_DIR
    export PATH="$MINICONDA3_PATH/bin:${PATH}"

    # mkdir $ARTIFACT_DIR
    #if [[ ! -d ${CMAKE_BUILD_DIR} ]]; then mkdir $CMAKE_BUILD_DIR; fi 
    echo "============ls current dir======="
    ls -al

    echo "========pwd of starting dir========"
    pwd

    echo "===========ls -al ../ from starting dir============="
    ls -al ../ 

    echo "==========echo local channel dir========="
    echo $CONDA_CHANNEL_DIR

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


}

if ! is_test ; then 
	main $@
fi
