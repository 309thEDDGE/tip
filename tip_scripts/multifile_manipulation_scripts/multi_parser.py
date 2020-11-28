from xml.dom import minidom
import subprocess
import os
import sys
import argparse
import yaml
import copy
import shutil
from pathlib import Path
from tip_scripts.exec import Exec

class MultiParser:
	def __init__(self, overwrite, tip_root_path, datafiles_path, dts_database_path, output_path):
		self.datafiles_path = datafiles_path
		self.dts_database_path = dts_database_path
		self.output_path = output_path
		self.tip_root_path = tip_root_path
		self.parser_executable_path = Path("")
		self.translator_executable_path = Path("")
		self.parser_output_path = Path("")
		self.translator_output_path = Path("")
		self.combined_metadata = []
		self.success_failure_list = {}
		self.overwrite = overwrite

	def initialize_paths(self):
		self.parser_executable_path = os.path.join(self.tip_root_path,'bin','tip_parse.exe')
		self.translator_executable_path = os.path.join(self.tip_root_path,'bin','tip_translate.exe')

		# check dts path exists
		if not os.path.isdir(self.dts_database_path):
			sys.exit('dts database path {} does not exist\n'.format(self.dts_database_path))

		# check if datafiles.xml exists
		if not os.path.isfile(self.datafiles_path):
			sys.exit('datafiles.xml file {} does not exist\n'.format(self.output_path))

		# check if output path exists
		if not os.path.isdir(self.output_path):
			sys.exit('output path {} does not exist\n'.format(self.output_path))

		self.parser_output_path = os.path.join(self.output_path,'parquet_data')
		self.translator_output_path = os.path.join(self.output_path,'parquet_data')

		# check if executables exists
		if not os.path.isfile(self.parser_executable_path):
			sys.exit('parser exe does not exist {}\n'.format(self.parser_executable_path))

		if not os.path.isfile(self.translator_executable_path):
			sys.exit('translator exe does not exist {}\n'.format(self.translator_executable_path))

		# If the translator_data directory doesn't exist, create it
		if not os.path.isdir(self.translator_output_path):
			print('creating directory {}'.format(translator_output_path))
			os.mkdir(translator_output_path)

		# if the parser_data directory doesn't exist, create it
		if not os.path.isdir(self.parser_output_path):
			print('creating directory {}'.format(parser_output_path))
			os.mkdir(parser_output_path)

		# update the working directly to bin for the executable to find
		# config files
		os.chdir(os.path.join(self.tip_root_path,'bin'))

	def remove_parsed(self, parser_output_path, chapter10name, translated_path):

		parsed_path = os.path.join(parser_output_path, str(chapter10name) + '_1553.parquet')
		ethernet_path = os.path.join(parser_output_path, str(chapter10name) + '_ethernet.parquet')

		#delete parsed 1553 if present
		if os.path.isdir(parsed_path):
			print('Deleting parsed directory \n{}\n'.format(parsed_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  parsed_path
			shutil.rmtree(delete_path)

		#delete ethernet if present
		if os.path.isdir(ethernet_path):
			print('Deleting ethernet directory \n{}\n'.format(ethernet_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  ethernet_path
			shutil.rmtree(delete_path)

		# remove translated if present
		if os.path.isdir(translated_path):
			print('Deleting translated directory \n{}\n'.format(translated_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  translated_path
			shutil.rmtree(delete_path)

	def remove_translated(self, translated_path):

		# remove translated if exists
		if os.path.isdir(translated_path):
			print('Deleting translated directory \n{}\n'.format(translated_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  translated_path
			shutil.rmtree(delete_path)

	def parse_and_translate(self):

		datafiles_xml = minidom.parse(str(self.datafiles_path))
		datafiles = datafiles_xml.getElementsByTagName('dataFile')


		for datafile in datafiles:
			print('\n\n\n\n')
			translator_success = False
			ch10path = datafile.attributes['Path'].value
			head, tail = os.path.split(ch10path)
			filename, ext = os.path.splitext(ch10path)
			chapter10name, ext = os.path.splitext(tail)
			parsed_path = os.path.join(self.parser_output_path, str(chapter10name) + '_1553.parquet')
			translated_path = os.path.join(self.translator_output_path, str(chapter10name) + '_1553_translated')

			# Initialize combined metadata
			self.combined_metadata.append({'ch10path': ch10path,  
									'1553_parquet_path': parsed_path, 
									'1553_translated_path': '', 
									'ch10name': chapter10name,
									'Date': datafile.attributes['Date'].value, 
									'Size': datafile.attributes['Size'].value, 
									'TimeOfDay': datafile.attributes['TimeOfDay'].value, 
									'TailNum': datafile.attributes['TailNum'].value, 
									'Block': datafile.attributes['Block'].value, 
									'DTS_name': datafile.attributes['AircraftCometName'].value, 
									'SoftwareVersion': datafile.attributes['AircraftTapeName'].value, 
									'Pilot': datafile.attributes['Pilot'].value, 
									'CallSign': datafile.attributes['CallSign'].value, 
									'Parsed': False, 
									'Translated': False 
									})


			print('---------chapter10 name: {}---------\n'.format(tail))

			# check if the file is a chapter 10 file
			if ext != '.ch10':
				print('\n\n--Parser Failure!!\nfilename {} is not a chapter 10'.format(ch10path))
				self.remove_parsed(self.parser_output_path, chapter10name,translated_path)
				continue

			# check if the chapter 10 exists
			if not os.path.isfile(ch10path):
				print('\n\n--Parser Failure!!\nch10 path {} does not exist'.format(ch10path))
				self.remove_parsed(self.parser_output_path, chapter10name, translated_path)
				continue

			print('---Parse--- \n{}'.format(ch10path))
			

			# if overwrite is specified, delete old parsed folder
			if self.overwrite:
				if os.path.isdir(parsed_path):
					self.remove_parsed(self.parser_output_path, chapter10name,translated_path)

			# if the parsed path exists and overwrite is not
			# specified don't run the parser again
			if not (os.path.isdir(parsed_path) and not self.overwrite):
				execParse = Exec()
				execParse.exec_list([str(self.parser_executable_path), str(ch10path), str(self.parser_output_path)])
				stdout, stderr = execParse.get_output()				
				print('Parsed Path: {}'.format(parsed_path))
				if stdout.find('Duration:') == -1:
					print('\n\n--Parser Failure!!')
					self.remove_parsed(self.parser_output_path, chapter10name,translated_path)
					continue

			else:
				print('\nExistant parsed file, specify --overwrite to re-parse \n{}\n'.format(self.parser_output_path))

			if not os.path.isdir(parsed_path):
				print('\n\n--Parser Failure!!\nexpected parser output: {}'.format(parsed_path))
				continue
				
			print('SUCCESS')
			self.combined_metadata[-1]['Parsed'] = True
			print('Parser Success:\n {}\n\n'.format(parsed_path))


			# start translation routine
			print('---Translate--- \n{}'.format(parsed_path))

			# check if parsed file exists
			if not os.path.isdir(parsed_path):
				print('\n\n--Translator Failure!!\nparsed file {} does not exist'.format(parsed_path))
				self.remove_translated(translated_path)
				continue

			DTS_path_yaml = os.path.join(self.dts_database_path,datafile.attributes['AircraftCometName'].value + '_ICD.yaml')
			DTS_path_txt = os.path.join(self.dts_database_path,datafile.attributes['AircraftCometName'].value + '_ICD.txt')
			DTS_path = Path('')
			if os.path.isfile(DTS_path_yaml):
				DTS_path = DTS_path_yaml

			elif os.path.isfile(DTS_path_txt):
				DTS_path = DTS_path_txt

			else:
				print('Neither txt nor yaml DTS paths exist\ntxt: {}\nyaml: {}\n'.format(DTS_path_txt,DTS_path_yaml))
				self.remove_translated(translated_path)
				continue
			

			# if overwrite is specified, delete old translated folder
			if self.overwrite:
				if os.path.isdir(translated_path):					
					self.remove_translated(translated_path)

			# if the translated path exists and overwrite is not
			# true don't run the parser again
			if  not (os.path.isdir(translated_path) and not self.overwrite):
				execTranslate = Exec()
				arg_list = [str(self.translator_executable_path), str(parsed_path), str(DTS_path)]
				execTranslate.exec_list(arg_list)
				stdout, stderr = execTranslate.get_output()
				
				if stdout.find('Duration:') == -1:
					print('\n\n--Translator Failure!!')

					# when failure delete translated folder if still there
					self.remove_translated(translated_path)
					continue

			else:
				print('\nExistant translated directory, specify --overwrite to re-translate \n{}\n'.format(self.translator_output_path))

			if not os.path.isdir(translated_path):
				print('\n\n--Translator Failure!!\nexpected translator output: {}'.format(translated_path))
				continue

			self.combined_metadata[-1]['Translated'] = True
			self.combined_metadata[-1]['1553_translated_path'] = translated_path

			print('Translator Success:\n {}\n'.format(translated_path))

	'''
	def delete_dir(self, element):
		if os.path.isdir(element):
			elements = os.listdir(element)
			for item in elements:
				item = os.path.join(element,item)
				self.delete_dir(item)
		else:
			os.remove(element)
			return

		os.rmdir(element)
	'''

	def write_metadata(self):
		
		print('\n\n---------Writing metadata---------\n')
		# only write metadata if translated or parsed 
		# data were written successfully
		sifted_combined_metadata = []

		failure_list = {'Parsed': [], 'Translated': []}

		# write meta_data to each file and save the metadata to 
		# sifted combined metadata if either parsed or translated
		# data were parsed
		for file_level_metadata in self.combined_metadata:
			pq_directory = file_level_metadata['1553_parquet_path']
			translated_directory = file_level_metadata['1553_translated_path']

			# only allow combined metadata to be written out if
			# either the translated data or parsed data are valid
			# and the directories exist
			if (file_level_metadata['Parsed'] and os.path.isdir(pq_directory)) or \
				(file_level_metadata['Translated'] and os.path.isdir(translated_directory)):
				sifted_combined_metadata.append(copy.deepcopy(file_level_metadata))

			# check for parser failure and add to failure statistics
			if not (file_level_metadata['Parsed'] and os.path.isdir(pq_directory)):
				failure_list['Parsed'].append(copy.deepcopy(file_level_metadata['ch10path']))

			# check for translator failure and add to failure statistics
			if  not (file_level_metadata['Translated'] and os.path.isdir(translated_directory)):
				failure_list['Translated'].append(copy.deepcopy(file_level_metadata['ch10path']))			

			# attempt to add file level metadata gathered from the xml for each parsed file
			if os.path.isdir(pq_directory):
				yaml_path = os.path.join(pq_directory,'_file_portion_metadata.yaml');

				# delete unnecessary keys for file level metadata
				if '1553_parquet_path' in file_level_metadata: del file_level_metadata['1553_parquet_path']
				if '1553_translated_path' in file_level_metadata: del file_level_metadata['1553_translated_path']			
				if 'Translated' in file_level_metadata: del file_level_metadata['Translated']
				if 'Parsed' in file_level_metadata: del file_level_metadata['Parsed']

				with open(yaml_path, 'w') as file:
					print('\nWriting file_portion_metadata: \n{}\n'.format(pq_directory))
					yaml.dump(file_level_metadata,file)


			else:
				print('Failed to write file_portion_metadata:\n{}\n'.format(pq_directory))

		# write combined meta_data to yaml
		combined_yaml_path = os.path.join(self.output_path,'_combined_metadata.yaml');
		with open(combined_yaml_path, 'w') as file:
			print('\n--Writing combined yaml meta_data: \n{}\n\n'.format(yaml_path))
			yaml.dump(sifted_combined_metadata,file)

		# write failure statistics to yaml
		failure_yaml_path = os.path.join(self.output_path,'_failure_stats.yaml');
		with open(failure_yaml_path, 'w') as file:
			print('\n--Writing failure status to yaml file: \n{}\n\n'.format(failure_yaml_path))
			yaml.dump(failure_list,file)

		print('\n\n---Finished---\n\n')


	def exec(self):		

		self.initialize_paths()

		self.parse_and_translate()

		self.write_metadata()		