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
	

# Creating a new mirror_config.yaml file

If another environment needs to be mirrored, use the `config_creator.py` file to write out the configuration to `mirror_config.py`, replacing <env> with the local environment that needs to be mirrored:

	python config_creator.py <env>

Providing an optional filepath will output to a different file name:

	python config_creator.py <env> <outpath>
