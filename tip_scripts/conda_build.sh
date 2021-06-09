#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)

main() {
    echo -n "Setting up conda build channel..."
    mkdir $CONDA_CHANNEL_DIR
    
    conda index $CONDA_CHANNEL_DIR -s linux-64 -s noarch
    echo -n "Building recipes"
    cd $SCRIPT_PATH/conda-recipes
    conda build spdlog gtest libtins yaml-cpp --croot $CONDA_CHANNEL_DIR
    conda build tip -c $CONDA_CHANNEL_DIR --croot $CONDA_CHANNEL_DIR
}

main $@
