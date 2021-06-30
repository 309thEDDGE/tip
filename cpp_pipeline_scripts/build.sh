#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
	set_exit_on_error
	setup
	
    echo -n "Installing Miniconda"
    export PATH="/home/user/miniconda3/bin:${PATH}"
    dnf install wget -y
    wget \
         https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
         && bash Miniconda3-latest-Linux-x86_64.sh -b -p /home/user/miniconda3 \
         && rm -f Miniconda3-latest-Linux-x86_64.sh 	

    pip install conda-mirror    
    echo -n "Installing conda-build"
    conda install conda-build -y
    echo -n "Change directory to conda-build recipes"
    cd tip_scripts
    echo -n "Directory list"
    ls
    echo -n "Building tip"
    export CONDA_CHANNEL_DIR="/local-channel"
    export CONDA_MIRROR_DIR="/local-mirror"
    export MIRROR_CONFIG="conda-mirror/mirror_config.yaml"
    ./conda_build.sh
    ./conda-mirror/clone.sh

} # main



# ------------------ RUN MAIN ---------------------
if ! is_test ; then 
	main $@
fi
