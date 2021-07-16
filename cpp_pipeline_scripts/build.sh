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

    mkdir test_dir
    echo "proof1" > test_dir/test_cache.txt

    mkdir -p $ARTIFACT_DIR
    cp -r test_dir/ $ARTIFACT_DIR/


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

    cd $SCRIPT_START_DIR

    echo "testing copy of second test file"
    echo "proof2" > test_dir/test_cache2.txt

    cp -r test_dir/test_cache2.txt $ARTIFACT_DIR/

    echo "testing ability to save tar files"
    tar -cvf test_dir.tar ./test_dir
    cp test_dir.tar $ARTIFACT_DIR

    echo "listing artifact DIR after test tar copy"
    ls -hl $ARTIFACT_DIR
    
    # mkdir $CONDA_CHANNEL_DIR
    # echo "proof2" > $CONDA_CHANNEL_DIR/test_cache.txt

    # cp /home/user/miniconda3/conda-bld/linux-64/click-8.0.1-py38_0.tar.bz2 $ARTIFACT_DIR/
    # cp $CONDA_CHANNEL_DIR/test_cache.txt $ARTIFACT_DIR/
    
    tar -cvf local_channel.tar /local-channel

    echo "listing and grepping for local_channel.tar"
    ls -hl | grep "local_channel.tar"

    echo "copying tarball to artifact dir"
    cp local_channel.tar $ARTIFACT_DIR

    echo "show all contents of artifact dir"
    ls -hl $ARTIFACT_DIR
    
    # echo "creating artifact dir if not already present"
    # if [[ ! -d ${ARTIFACT_DIR} ]]; then mkdir -p $ARTIFACT_DIR; fi

    # echo "listing channel that was created"
    # ls -al /home/user/miniconda3/conda-bld/linux-64

    # echo "list with wildcard for tar"
    # ls -al /home/user/miniconda3/conda-bld/linux-64/*.tar*
    
    # echo "attempting copy of tarball"
    # cp /home/user/miniconda3/conda-bld/linux-64/*.tar* $ARTIFACT_DIR/

    # echo "list artifact dir"
    # ls $ARTIFACT_DIR

    # echo "copying test text file"
    # echo "proof positive" > test.txt
    # cp test.txt $ARTIFACT_DIR

    # echo "copying test tar file"
    # tar -cvf test_tar.tar test.txt
    # cp test_tar.tar $ARTIFACT_DIR
}

if ! is_test ; then 
	main $@
fi
