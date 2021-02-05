#!/usr/bin/env bash

SCRIPT_PATH=$(dirname $0)
source $SCRIPT_PATH/setup.sh

main() {
	set_exit_on_error
	setup
	
	# If we are running on a pipeline, always build with Alkemist
	# If not, only include Alkemist if ALKEMIST_LICENSE_KEY is provided
	if is_alkemist_included; then
		# Add LFR ALKEMIST build flags
		echo "Building with Alkemist libraries"
		export TRAPLINKER_EXTRA_LDFLAGS="--traplinker-static-lfr -L${DEPS_DIR}/alkemist-lfr/lib"
		OLDPATH="${PATH}"
		export PATH="${LFR_ROOT_PATH}/scripts:${PATH}"
	else
		echo "Building WITHOUT Alkemist libraries:"
		echo "   not on a pipeline and ALKEMIST_LICENSE_KEY unset or empty"
	fi

	echo -n "Checking for ninja..."
	if [[ -f /usr/local/bin/ninja ]] ; then
		echo "yes"
		CMAKE="cmake -G Ninja"
		MAKE=ninja
	else
		echo "no.  Using make"
		CMAKE="cmake"
		MAKE="make -j8"
	fi

	# Get paths to all cached libraries
	BINARIES=( $(find $BUILD_DIR -name \*.a) ) || BINARIES=""
	### Diagnostics
	#  echo "Found ${#BINARIES[*]} cached libraries: ${BINARIES[*]}"
	#  [ ${#BINARIES[*]} -gt 0 ] && ls -lt ${BINARIES[*]}
	### End

	# If no cached libraries exist, skip directly to build
	if [ -z ${BINARIES[0]} ]; then
		echo "No cached libraries; doing a clean build"
	# If variable TIP_REBUILD_ALL is defined, rebuild all
	elif [ -v TIP_REBUILD_ALL ]; then
		echo "Variable TIP_REBUILD_ALL is set; rebuilding all"
		echo rm ${BINARIES[*]}
		rm ${BINARIES[*]}
	# Otherwise, allow CMake to build based on modification times
	else
		echo "Checking for outdated binaries"
		echo "...setting each source file mod time to its last commit time"
		cd $BASE_DIR
		for FILE in $(git ls-files | grep -e "\.cpp$\|\.h\|\.sh$")
		do
			TIME=$(git log --pretty=format:%cd -n 1 --date=iso -- "$FILE")
			TIME=$(date -d "$TIME" +%Y%m%d%H%M.%S)
			touch -m -t "$TIME" "$FILE"
			echo -n .
		done
		echo "Done"

		# CMake doesn't know about build.sh, so check its dependencies explicitly
		for file in ${BINARIES[*]}; do
			[ $BUILD_SCRIPT -nt $file ] && rm $file && echo "...removed outdated $file"
		done
	fi

	echo "Running '$CMAKE'"
	# the pipeline build image has a /deps directory
	# if there is a /deps directory then replace the local deps directory
	if [ -d $DEPS_SOURCE ] ; then
		echo "Restoring 3rd party dependencies from $DEPS_SOURCE"
		rm -rf $DEPS_DIR
		mv $DEPS_SOURCE $DEPS_DIR
	fi

	mkdir -p $BUILD_DIR
	cd $BUILD_DIR
	$CMAKE -DLIBIRIG106=ON -DVIDEO=ON ..

	echo "Running '$MAKE'"
	$MAKE -v install
	# move bin folder to build for use in later pipeline stages
	cd $BASE_DIR
	if [ -d bin ] ; then 
		rm -rf build/bin
		cp -r bin build/
	fi

	PATH=$"{OLDPATH}"
} # main



# ------------------ RUN MAIN ---------------------
if ! is_test ; then 
	main $@
fi
