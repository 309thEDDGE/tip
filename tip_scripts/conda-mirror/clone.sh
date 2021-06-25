#!/bin/sh

conda-mirror \
    --target-directory $CONDA_MIRROR_DIR \
    --platform linux-64 \
    --config $MIRROR_CONFIG \
    --upstream-channel https://repo.anaconda.com/pkgs/main/

conda-mirror \
    --target-directory $CONDA_MIRROR_DIR \
    --platform noarch \
    --config $MIRROR_CONFIG \
    --upstream-channel https://repo.anaconda.com/pkgs/main/ 
