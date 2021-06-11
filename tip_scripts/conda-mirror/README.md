# Creating local channel with required tip packages
This guide assumes that the tip dependencies as well as tip package
have been built locally.

The included `mirror_config.yaml` has a whitelist of all the required packages for tip to solve. 

To create a local channel in the current directory names `local_channel` first install the conda mirror package via conda:

	`conda install -c conda-forge conda-mirror`

Then run the bash script to download the dependent packages from the anaconda channel:

	`sh clone.sh`
	
This local channel can then be used in combination with the path where tip was built to install tip. Note that `--override-channels` exclusively will look for conda packages in channels specified. `<tip_build_path>` is replaced with the path to the local channel where tip was built:

	conda install tip -c <tip_build_path> -c ./local_mirror --override-channels
	
