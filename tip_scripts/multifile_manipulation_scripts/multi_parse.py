from xml.dom import minidom
import subprocess
import os
import sys
import argparse
import yaml
from pathlib import Path


tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(tip_root_path)


from tip_scripts.exec import Exec


if __name__ == '__main__':

	if len(sys.argv) < 4:
		sys.exit('three arguments required!')

	datafiles_path = sys.argv[1] #xml path
	output_path = sys.argv[2] # base path for parquet output
	dts_base_path = sys.argv[3]


	parser_executable_path = os.path.join(tip_root_path,'bin','tip_parse.exe')
	translator_executable_path = os.path.join(tip_root_path,'bin','tip_translate.exe')


	# Check if output path exists
	if not os.path.isdir(output_path):
		sys.exit('output path {} does not exist\n'.format(output_path))

	parser_output_path = os.path.join(output_path,'parquet_data')
	translator_output_path = os.path.join(output_path,'parquet_data')

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
	combined_metadata = []

	for datafile in datafiles:
		print('\n\n\n\n')
		parser_success = False
		translator_success = False
		ch10path = datafile.attributes['Path'].value
		head, tail = os.path.split(ch10path)
		filename, ext = os.path.splitext(ch10path)
		chapter10name, ext = os.path.splitext(tail)

		print('chapter10 name: {}'.format(tail))

		# check if the file is a chapter 10 file
		if ext != '.ch10':
			print('\n\n--Parser Failure!!\nfilename {} is not a chapter 10'.format(ch10path))
			continue

		# check if the chapter 10 exists
		if not os.path.isfile(ch10path):
			print('\n\n--Parser Failure!!\nch10 path {} does not exist'.format(ch10path))
			continue



		print('---Parse {}'.format(ch10path))
		execParse = Exec()
		execParse.exec_list([str(parser_executable_path), str(ch10path), str(parser_output_path)])
		stdout, stderr = execParse.get_output()
		parsed_path = os.path.join(parser_output_path, str(chapter10name) + '_1553.parquet')
		print('Parsed Path: {}'.format(parsed_path))
		if stdout.find('Duration:') == -1:
			print('\n\n--Parser Failure!!')
			continue

		if not os.path.isdir(parsed_path):
			print('\n\n--Parser Failure!!\nexpected parser output: {}'.format(parsed_path))
			continue
			
		print('SUCCESS')
		parser_success = True
		# initialize set
		combined_metadata.append({'ch10path': ch10path,  
								'1553_parquet_path': parsed_path, 
								'1553_translated_path': '', 
								'Date': datafile.attributes['Date'].value, 
								'Size': datafile.attributes['Size'].value, 
								'TimeOfDay': datafile.attributes['TimeOfDay'].value, 
								'TailNum': datafile.attributes['TailNum'].value, 
								'Block': datafile.attributes['Block'].value, 
								'DTS_name': datafile.attributes['AircraftCometName'].value, 
								'SoftwareVersion': datafile.attributes['AircraftTapeName'].value, 
								'Pilot': datafile.attributes['Pilot'].value, 
								'CallSign': datafile.attributes['CallSign'].value, 
								'Parsed': True, 
								'Translated': False 
								})

		print('Parser Success:\n {}\n\n'.format(parsed_path))


		# Start Translation routine
		DTS_path_yaml = os.path.join(dts_base_path,datafile.attributes['AircraftCometName'].value + '_ICD.yaml')
		DTS_path_txt = os.path.join(dts_base_path,datafile.attributes['AircraftCometName'].value + '_ICD.txt')
		DTS_path = Path('')
		if os.path.isfile(DTS_path_yaml):
			DTS_path = DTS_path_yaml

		elif os.path.isfile(DTS_path_txt):
			DTS_path = DTS_path_txt

		else:
			print('Neither txt nor yaml DTS paths does not exist\ntxt: {}\nyaml: {}\n'.format(DTS_path_txt,DTS_path_yaml))
			continue


		execTranslate = Exec()
		arg_list = [str(translator_executable_path), str(parsed_path), str(DTS_path)]
		execTranslate.exec_list(arg_list)
		stdout, stderr = execTranslate.get_output()
		translated_path = os.path.join(translator_output_path, str(chapter10name) + '_1553_translated')
		if stdout.find('Duration:') == -1:
			print('\n\n--Translator Failure!!')
			continue

		if not os.path.isdir(translated_path):
			print('\n\n--Translator Failure!!\nexpected translator output: {}'.format(translated_path))
			continue

		combined_metadata[-1]['Translated'] = True
		combined_metadata[-1]['1553_translated_path'] = translated_path

		print('Translator Success:\n {}\n\n'.format(translated_path))

	# write combined meta_data to yaml
	yaml_path = os.path.join(output_path,'combined_metadata.yaml');
	with open(yaml_path, 'w') as file:
		print('\n--Writing combined yaml meta_data: \n{}\n\n'.format(yaml_path))
		yaml.dump(combined_metadata,file)

	# write meta_data to each file
	for file_level_metadata in combined_metadata:
		pq_directory = file_level_metadata['1553_parquet_path']
		if os.path.isdir(pq_directory):
			yaml_path = os.path.join(pq_directory,'file_portion_metadata.yaml');
			# delete unnecessary keys for file level metadata
			if '1553_parquet_path' in file_level_metadata: del file_level_metadata['1553_parquet_path']
			if 'Translated' in file_level_metadata: del file_level_metadata['Translated']
			if 'Parsed' in file_level_metadata: del file_level_metadata['Parsed']
			with open(yaml_path, 'w') as file:
				print('\nWriting file_portion_metadata: \n{}\n\n'.format(pq_directory))
				yaml.dump(file_level_metadata,file)
		else:
			print('Failed to write file_portion_metadata:\n{}\n'.format(pq_directory))

	print('\n\n---Finished---\n\n')


