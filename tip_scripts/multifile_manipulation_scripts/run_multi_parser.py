import os
import sys

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
												 'datafiles.xml file. Save local parquet database.')
	aparse.add_argument('datafiles_path', metavar='<datafiles.xml path>', type=str, 
						help='Full path to datafiles xml file for a list of '
						' chapter 10 files to be parsed and translated')

	aparse.add_argument('dts_database_path', metavar='<DTS database path>', type=str, 
						help='Path to folder where DTS files are located')

	aparse.add_argument('output_path', metavar='<Output Path>', type=str, 
						help='Full path to location where parsed results will be stored')

	aparse.add_argument('--overwrite', action='store_true', default=False, 
						help='Overwrite existing parsed and translated output')

	args = aparse.parse_args()


	datafiles_path = args.datafiles_path
	dts_database_path = args.dts_database_path
	output_path = args.output_path
	overwrite = args.overwrite

	multi_parser = MultiParser(overwrite, tip_root_path, datafiles_path, dts_database_path, output_path)

	multi_parser.exec()
	




