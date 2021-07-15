#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
    set_exit_on_error
    setup
    export MINICONDA3_PATH="/home/user/miniconda3"
    export ARTIFACT_DIR="${ARTIFACT_FOLDER}/build-metadata/build-artifacts"
    # export CONDA_CHANNEL_DIR="/local-channel"
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

    # tar -cvf local_channel.tar /local-channel
    
    echo "creating artifact dir if not already present"
    if [[ ! -d ${ARTIFACT_DIR} ]]; then mkdir -p $ARTIFACT_DIR; fi

    echo "listing channel that was created"
    ls -al /home/user/miniconda3/conda-bld/linux-64

    echo "list with wildcard for tar"
    ls -al /home/user/miniconda3/conda-bld/linux-64/*.tar*
    
    echo "attempting copy of tarball"
    cp /home/user/miniconda3/conda-bld/linux-64/*.tar* $ARTIFACT_DIR/

    echo "list artifact dir"
    ls $ARTIFACT_DIR

    echo "copying test text file"
    echo "proof positive" > test.txt
    cp test.txt $ARTIFACT_DIR

    echo "copying test tar file"
    tar -cvf test_tar.tar test.txt
    cp test_tar.tar $ARTIFACT_DIR
}

if ! is_test ; then 
	main $@
fi
