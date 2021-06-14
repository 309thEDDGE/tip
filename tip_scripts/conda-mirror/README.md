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
	

# Creating a mirror_config.yaml file

TODO: Create a python script to automate the below

This method leverages python and the pyyaml packate. To create the
config file, run `conda list` and copy the sting output into a python
variable called json_str:

```
json_str = """{
  "channels": [
    "local",
    "defaults"
  ],
  "dependencies": [
    "_libgcc_mutex=0.1=main",
    "_openmp_mutex=4.5=1_gnu",
    "arrow-cpp=4.0.1=py39hced866c_3",
...
"""
```

Then run the following code to process the string:

```
whitelist = """blacklist:
    - name: \"*\"
whitelist:\n"""


for entry in json.loads(json_str)['dependencies']:
    parts = entry.split('=')
    whitelist += f"""    - name: \"{parts[0]}\"
      version: \"{parts[1]}\"\n"""
	  
print(whitelist)
```

This can be directly copied into a file with the name `mirror_config.yaml`
