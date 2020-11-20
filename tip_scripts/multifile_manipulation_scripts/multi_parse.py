from xml.dom import minidom
import subprocess
import os
import sys
import argparse
from pathlib import Path


tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(tip_root_path)


from tip_scripts.exec import Exec


if __name__ == '__main__':

	'''
		aparse = argparse.ArgumentParser(description='Description')
		aparse.add_argument('truth_dir', metavar='<truth dir. path>', type=str, 
							help='Full path to directory containing source Ch10 files, '
							'previously generated parsed and translated output data, '
							'an ICD to be used during the 1553 translation step for each '
							'ch10 file and a ch10list.csv file which matches the ch10 name '
							'to the ICD to be used during translation. ch10list.csv format:\n'
							'<ch10 name 1> <icd name for ch10 1> # both present in the truth direcotry\n'
							'<ch10 name 2> <icd name for ch10 2>\n'
							'...')
		aparse.add_argument('test_dir', metavar='<test dir. path>', type=str, 
							help='Full path to directory in which freshly generated output data shall be '
							'placed. TIP parse and translate routines will be applied to each of the ch10 '
							'files found in the truth directory and placed in the test dir.')
		aparse.add_argument('log_dir', metavar='<log output dir. path>', type=str, 
							 help='Full path to directory in which logs shall be written')
		aparse.add_argument('-l', '--log-string', type=str, default=None,
							help='Provide a string to be inserted into the log name')
		aparse.add_argument('-v', '--video', action='store_true', default=False,
							help='Generate raw video parquet files (validation not implemented)')
		aparse.add_argument('--no-yaml', action='store_true', default=False, 
							help='Turn off Yaml dependency import (deepdiff) and do not compare yaml files')
	'''

	datafiles_path = Path(r'C:\Users\A10\Documents\Source\locker\mutli_file_manipulation\datafiles.xml')
	output_path = Path(r'C:\Users\A10\Documents\Source\locker\mutli_file_manipulation\multi_files')


	parser_executable_path = os.path.join(tip_root_path,'bin','tip_parse.exe')
	translator_executable_path = os.path.join(tip_root_path,'bin','tip_translate.exe')


	# Check if output path exists
	if not os.path.isdir(output_path):
		sys.exit('output path {} does not exist\n'.format(output_path))

	parser_output_path = os.path.join(output_path,'parser_data')
	translator_output_path = os.path.join(output_path,'translator_data')

	# If the parser_data directory doesn't exist, create it
	if not os.path.isdir(parser_output_path):
		print('creating directory {}'.format(parser_output_path))
		os.mkdir(parser_output_path)

	# check if executables exist
	if not os.path.isfile(parser_executable_path):
		sys.exit('parser exe does not exist {}\n'.format(parser_executable_path))

	if not os.path.isfile(translator_executable_path):
		sys.exit('translator exe does not exist {}\n'.format(translator_executable_path))

	# If the translator_data directory doesn't exist, create it
	if not os.path.isdir(translator_output_path):
		print('creating directory {}'.format(translator_output_path))
		os.mkdir(translator_output_path)

	# update the working directly to bin for the executable to find
	# config files
	os.chdir(os.path.join(tip_root_path,'bin'))


	datafiles_xml = minidom.parse(str(datafiles_path))
	datafiles = datafiles_xml.getElementsByTagName('dataFile')

	for datafile in datafiles:
		ch10path = datafile.attributes['Path'].value
		print('---Parse {}'.format(ch10path))
		execParse = Exec()
		execParse.exec_list([str(parser_executable_path), str(ch10path), str(parser_output_path)])
		stdout, stderr = execParse.get_output()
		if stdout.find('Duration:') != -1:
			print('SUCCESS')
		else:
			print('FAILURE')
		
		#output = subprocess.run([parser_executable_path, ch10path, parser_output_path], capture_output=True)
		#print(output)


# Check if 