#!/usr/bin/env bash
echo "-----------showing working dir-------------"
pwd
echo "-----------showing files in current dir----"
ls

mkdir build
echo "-----------seconds showing files in current dir----"
ls

touch ./build/cache_proof.txt


# SCRIPT_PATH=$(dirname $0)
# source $SCRIPT_PATH/setup.sh

# main() {
# 	set_exit_on_error
# 	setup
	
#     echo -n "Installing Miniconda"
#     export MINICONDA3_PATH="/home/user/miniconda3"
#     export CONDA_CHANNEL_DIR="/local-channel"
#     export PATH="$MINICONDA3_PATH/bin:${PATH}"
#     dnf install wget -y
#     wget \
#          https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh \
#          && bash Miniconda3-latest-Linux-x86_64.sh -b -p $MINICONDA3_PATH

#     echo -n "Installing conda-build"
#     conda install conda-build -y
#     echo -n "Change directory to conda-build recipes"
#     cd tip_scripts
#     echo -n "Building tip"
#     ./conda_build.sh
# }

# if ! is_test ; then 
# 	main $@
# fi
