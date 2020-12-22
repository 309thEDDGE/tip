import os
import sys
import yaml

tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(tip_root_path)

from tip_scripts.multifile_manipulation_scripts.multi_parser import MultiParser

# Argument parser
import argparse


if __name__ == '__main__':

	#
	# Setup command line arguments
	#
	aparse = argparse.ArgumentParser(description='Parse multiple chapter 10 files using the '
												 'datafiles.xml file. Save local parquet database.'
												 'Move multi_parse_conf.yaml from conf/default_conf to'
												 'conf/ and adjust config options as needed.')
	aparse.add_argument('datafiles_path', metavar='<datafiles.xml path>', type=str, 
						help='Full path to datafiles xml file for a list of '
						' chapter 10 files to be parsed and translated')

	aparse.add_argument('dts_database_path', metavar='<DTS database path>', type=str, 
						help='Path to folder where DTS files are located')

	aparse.add_argument('output_path', metavar='<Output Path>', type=str, 
						help='Full path to location where parsed results will be stored')

	args = aparse.parse_args()


	datafiles_path = args.datafiles_path
	dts_database_path = args.dts_database_path
	output_path = args.output_path

	# gather yaml config data
	config_file_path = os.path.join(tip_root_path, 'conf', 'multi_parse_conf.yaml')

	if not os.path.isfile(config_file_path):
		sys.exit('config file not present {}'.format(config_file_path))

	config_data = {}

	with open(config_file_path) as file:
		config_data = yaml.full_load(file)

	multi_parser = MultiParser(config_data, tip_root_path, datafiles_path, dts_database_path, output_path)

	multi_parser.exec()
	




