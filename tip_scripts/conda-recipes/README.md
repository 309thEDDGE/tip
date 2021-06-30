# Building conda packages

Note that this guide is indended for use on Linux based
systems. Windows systems have not been tested.

The structure of the conda package config files as well as other
packagind documentation can be found here:
https://docs.conda.io/projects/conda-build/en/latest/resources/define-metadata.html

To build a conda package from any of the directories here, conda must
be installed as well as the conda build package
https://anaconda.org/anaconda/conda-build

With conda build installed, any of these packages can be installed
with the following:

	conda build <dir>
	
To build spdlog for example, change directories into
/tip/operations/conda-builds and then run the following:

	conda build spdlog
	
When this completes, it will output a tarball to your anaconda root
directory. The conda build output will indicate where the tarball
resides.

# Creating a local conda channel

To use the try out the local tarballs a local conda channel can be
created.

Here are the steps to create a blank directory and build a conda
channel in it.

	mkdir local-channel
	cd local-channel
	conda index -s linux-64 -s noarch

With this channel built, we then need to transfer over the tarballs that were created during packaging up. See following example

	cp /home/tpotts/anaconda3/conda-bld/linux-64/spdlog-1.8.5-h6bb024c_0.tar.bz2 linux-64/
	
After this tarball is created, we need to re-index the channel by
running this command again:

	conda index -s linux-64 -s noarch
	
This will generate several metadata files indexing the packages and
versions available in the channel.

This directory is now setup to be an html server. Assuming python 3.x+ is installed, we can start an html server with:

	python -m http.server

The conda channel is now accessable from the local machine by providing the relevant local ip address and port when providing the conda channel for a package install.

