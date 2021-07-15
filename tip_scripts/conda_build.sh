#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)

main() {
    echo -n "Setting up conda build channel..."
    # mkdir -p $CONDA_CHANNEL_DIR
    
    conda index $CONDA_CHANNEL_DIR -s linux-64 -s noarch
    echo -n "Building recipes"
    cd $SCRIPT_PATH/conda-recipes
    # conda build deleteme-click -c conda-forge --croot $CONDA_CHANNEL_DIR
    conda build deleteme-click -c conda-forge
}

main $@
