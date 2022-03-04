#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
DIRECTORY_WHEN_EXECUTED=$(pwd)
TMP_CHANNEL=${HOME}/tmp-channel
CONDA_CHANNEL_DIR=${HOME}/local-channel

main() {

    echo -n "Setting up conda build channel..."

    echo -n "Building recipes"
    cd $SCRIPT_PATH/conda-recipes
    conda build tip -c conda-forge --croot ${TMP_CHANNEL}

    mkdir -p $CONDA_CHANNEL_DIR/linux-64
    mkdir -p $CONDA_CHANNEL_DIR/noarch
    mv $TMP_CHANNEL/linux-64/*tip*tar* $CONDA_CHANNEL_DIR/linux-64/
    #conda index $CONDA_CHANNEL_DIR -s linux-64 -s noarch

    
    cd $DIRECTORY_WHEN_EXECUTED
}

main $@
