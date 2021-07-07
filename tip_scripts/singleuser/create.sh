#!/usr/bin/env bash

main () {
	echo -n "Creating a vendored singleuser channel"
	cd tip_scripts/singleuser/
	python -m conda_vendor local_channels -f /tip/tip_scripts/singleuser/singleuser.yml -l $SINGLEUSER_CHANNEL_DIR
}

main $@
