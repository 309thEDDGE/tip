#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
DIRECTORY_WHEN_EXECUTED=$(pwd)

main() {

    echo -n "Setting up conda build channel..."

    echo -n "Building recipes"
    cd $SCRIPT_PATH/conda-recipes
    conda build tip -c conda-forge --croot ${HOME}/tmp-channel
    # conda build tip -c file:///local_channel --croot ${HOME}/tmp-channel

    mkdir -p $CONDA_CHANNEL_DIR/linux-64
    mkdir -p $CONDA_CHANNEL_DIR/noarch
    mv ${HOME}/tmp-channel/linux-64/*tip*tar* $CONDA_CHANNEL_DIR/linux-64/
    conda index $CONDA_CHANNEL_DIR -s linux-64 -s noarch

    
    cd $DIRECTORY_WHEN_EXECUTED
}

main $@
