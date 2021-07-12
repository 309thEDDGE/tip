#!/usr/bin/env bash

main () {
	echo -n "Mirroring upstream dependencies"
	conda-mirror \
	    --target-directory $CONDA_MIRROR_DIR \
	    --platform linux-64 \
	    --config $MIRROR_CONFIG \
	    --upstream-channel https://conda.anaconda.org/conda-forge/
	
	conda-mirror \
	    --target-directory $CONDA_MIRROR_DIR \
	    --platform noarch \
	    --config $MIRROR_CONFIG \
	    --upstream-channel https://conda.anaconda.org/conda-forge/
}

main $@
