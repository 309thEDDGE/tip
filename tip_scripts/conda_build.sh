#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
DIRECTORY_WHEN_EXECUTED=$(pwd)

main() {
    
    echo -n "Setting up conda build channel..."
    # mkdir -p $CONDA_CHANNEL_DIR
    
    conda index $CONDA_CHANNEL_DIR -s linux-64 -s noarch
    echo -n "Building recipes"
    cd $SCRIPT_PATH/conda-recipes
    conda build tip -c conda-forge --croot $CONDA_CHANNEL_DIR

    cd $DIRECTORY_WHEN_EXECUTED
}

main $@
