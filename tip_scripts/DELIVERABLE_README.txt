Run parse_and_translate.py to parse and translate
(for help run "python parse_and_translate.py -h")


Notes: 
1. 	Execute the script using Python 3.6 (higher versions may also work). If you have Python and numpy installed
    and know how to run a script then step 2. can be skipped. Execute the script and show the help menu by
	executing: python /path/to/parse_and_translate.py -h

2.	Run parse_and_translate.py from a new python enviroment (with numpy installed). Use these steps if you
	don't have Python or don't know how to use it.
	
	In Anaconda Shell (Download and install Anaconda3)
		conda create -n <enviroment name> python=3.6 numpy
		conda activate <enviroment name>
		python parse_and_translate.py -h

3. 	You don't need to move configuration files from default_conf, the first time you run parse_and_translate.py
	it will move the relevant config files from conf/default_conf to conf/. Leave all configuration files alone
	in conf/default_conf and only edit those in conf/

4.	Configuration files (see comments in config files for details on each config parameter)
		parse_conf.yaml
		translate_conf.yaml