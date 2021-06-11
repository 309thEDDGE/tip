#!/bin/sh

conda-mirror \
    --target-directory local_mirror \
    --platform linux-64 \
    --config mirror_config.yaml \
    --upstream-channel https://repo.anaconda.com/pkgs/main/

conda-mirror \
    --target-directory local_mirror \
    --platform noarch \
    --config mirror_config.yaml \
    --upstream-channel https://repo.anaconda.com/pkgs/main/ 
