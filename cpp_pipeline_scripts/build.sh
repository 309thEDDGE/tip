#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
	set_exit_on_error
	setup
	
    echo -n "Installing Miniconda"
    export PATH="/root/miniconda3/bin:${PATH}"
    dnf install wget -y
    wget \
        https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
        && mkdir /root/.conda \
        && bash Miniconda3-latest-Linux-x86_64.sh -b \
        && rm -f Miniconda3-latest-Linux-x86_64.sh 	

    echo -n "Checking for conda"
  
    if [[ -f /root/miniconda3/bin/conda ]] ; then 
        echo "Conda found"
    else
        echo "No Conda"
    fi
    
    echo -n "Installing conda-build"
    conda install conda-build -y
    pwd
    ls
    echo -n "Change directory to conda-build recipes"
    cd tip_scripts/conda-recipes
    echo -n "Directory list"
    ls
    echo -n "Building tip dependencies"
    conda build spdlog gtest libtins yaml-cpp
    echo -n "Building tip"
    conda build tip -c /root/miniconda3/conda-bld

} # main



# ------------------ RUN MAIN ---------------------
if ! is_test ; then 
	main $@
fi
