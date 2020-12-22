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
from datetime import datetime

class MultiParser:
	def __init__(self, overwrite, tip_root_path, datafiles_path, dts_database_path, output_path):
		self.datafiles_path = datafiles_path
		self.dts_database_path = dts_database_path
		self.output_path = output_path
		self.tip_root_path = tip_root_path
		self.parser_executable_path = Path("")
		self.translator_executable_path = Path("")
		self.combined_metadata = []
		self.overwrite = overwrite
		self.failure_stats = {'parser_failures': [], 'translator_failures': [], 'duplicate_ch10_file_paths': {}}


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

		self.parquet_data_path = os.path.join(self.output_path,'parquet_data')

		# check if executables exists
		if not os.path.isfile(self.parser_executable_path):
			sys.exit('parser exe does not exist {}\n'.format(self.parser_executable_path))

		if not os.path.isfile(self.translator_executable_path):
			sys.exit('translator exe does not exist {}\n'.format(self.translator_executable_path))

		# if the parquet_data directory doesn't exist, create it
		if not os.path.isdir(self.parquet_data_path):
			print('creating directory {}'.format(self.parquet_data_path))
			os.mkdir(self.parquet_data_path)

		# if failure stats folder does not exist, create it
		# if the parquet_data directory doesn't exist, create it
		failure_stats_dir = os.path.join(self.output_path,'failure_stats')
		if not os.path.isdir(failure_stats_dir):
			print('creating directory {}'.format(failure_stats_dir))
			os.mkdir(failure_stats_dir)


		# append time stamp to file name for failure stats yaml file
		now = datetime.now()
		# dd/mm/YY H:M:S
		dt_string = now.strftime("%m%d%Y_%H_%M_%S")
		# write failure statistics to yaml
		self.failure_yaml_path = os.path.join(failure_stats_dir,'_' + dt_string + '_failure_stats.yaml');

		# update the working directly to bin for the executable to find
		# config files
		os.chdir(os.path.join(self.tip_root_path,'bin'))

	def parse_and_translate(self):

		datafiles_xml = minidom.parse(str(self.datafiles_path))
		datafiles = datafiles_xml.getElementsByTagName('dataFile')
		ch10_names = {}


		for datafile in datafiles:
			# Continually overwrite the failure stats yaml file for continual
			# updates
			self.write_failure_stats()

			print('\n\n\n\n')
			translator_success = False
			ch10path = datafile.attributes['Path'].value
			head, tail = os.path.split(ch10path)
			filename, ext = os.path.splitext(ch10path)
			chapter10name, ext = os.path.splitext(tail)
			chapter10_database_folder_path = os.path.join(self.parquet_data_path,chapter10name)
			parsed_path = os.path.join(chapter10_database_folder_path, str(chapter10name) + '_1553.parquet')
			translated_path = os.path.join(chapter10_database_folder_path, str(chapter10name) + '_1553_translated')
			execute_translator_only = False

			# If the chapter 10 name has already been 
			# processed, add it to the failure statistics
			# and continue.
			if chapter10name in ch10_names.keys():
				if chapter10name not in self.failure_stats['duplicate_ch10_file_paths'].keys():
					self.failure_stats['duplicate_ch10_file_paths'][chapter10name] = []
					# make sure to insert the first chatper 10 path associated with the first chatper 10 found 
					self.failure_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10_names[chapter10name])
					self.failure_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10path)
				else:
					self.failure_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10path)
				continue

			ch10_names[chapter10name] = ch10path

			# Check if chapter 10 folder exists
			if os.path.isdir(chapter10_database_folder_path):
				# if overwrite = True, delete any old data
				if self.overwrite:
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					os.mkdir(chapter10_database_folder_path)
				else:
					# if overwrite = False, and parser data does not exist
					# delete and refresh chapter 10 data base folder
					# This is done to keep translation data consistent with
					# parser data
					if not os.path.isdir(parsed_path):
						self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
						os.mkdir(chapter10_database_folder_path)

					# if overwrite = False, and parser data does exist
					# and translator data does not exist,
					# only run the translator
					elif not os.path.isdir(translated_path):
						execute_translator_only = True
						print('Parser data exists without translation data for {}, only executing translation'.format(chapter10name))

					# if overwrite = False, and both parser and translator data
					# exist, simply continue, and leave data alone
					else:
						print('Data already exists for {}, skipping parse and translate steps'.format(chapter10name))
						continue

			# If chapter 10 folder does not
			# exist, create it. 
			else:
				os.mkdir(chapter10_database_folder_path)
			

			
			# Initialize combined metadata
			high_level_metadata = {'ch10path': ch10path,  
									'1553_parquet_path': parsed_path, 
									'1553_translated_path': translated_path, 
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
									}
			


			print('---------chapter10 name: {}---------\n'.format(tail))

			if not execute_translator_only:					

				# check if the file is a chapter 10 file
				if ext != '.ch10':
					print('\n\n--Parser Failure!!\nfilename {} is not a chapter 10'.format(ch10path))
					self.failure_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				# check if the chapter 10 exists
				if not os.path.isfile(ch10path):
					print('\n\n--Parser Failure!!\nch10 path {} does not exist'.format(ch10path))
					self.failure_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				print('---Parse--- \n{}'.format(ch10path))

				execParse = Exec()
				execParse.exec_list([str(self.parser_executable_path), str(ch10path), chapter10_database_folder_path])
				stdout, stderr = execParse.get_output()				
				print('Parsed Path: {}'.format(parsed_path))
				if stdout.find('Duration:') == -1:
					print('\n\n--Parser Failure!!')
					self.failure_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				if not os.path.isdir(parsed_path):
					print('\n\n--Parser Failure!!\nexpected parser output: {}'.format(parsed_path))
					self.failure_stats['parser_failures'].append(ch10path)
					continue
					
				print('SUCCESS')
				high_level_metadata['Parsed'] = True

				# write high level metadata
				high_level_metadata_yaml_path = os.path.join(chapter10_database_folder_path,'_high_level_metadata.yaml');
				with open(high_level_metadata_yaml_path, 'w') as file:
					print('\n--Writing _high_level_metadata: \n{}\n\n'.format(high_level_metadata_yaml_path))
					yaml.dump(high_level_metadata,file)

				print('Parser Success:\n {}\n\n'.format(parsed_path))


			# start translation routine
			print('---Translate--- \n{}'.format(parsed_path))

			# check if parsed file exists
			if not os.path.isdir(parsed_path):
				print('\n\n--Translator Failure!!\nparsed file {} does not exist'.format(parsed_path))
				self.remove_translated(translated_path)
				self.failure_stats['translator_failures'].append(ch10path)
				continue

			# Handle the case when execute_translator_only = True
			# and high_level_metadata['Parsed'] is never set to true
			high_level_metadata['Parsed'] = True

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
				self.failure_stats['translator_failures'].append(ch10path)
				continue

			execTranslate = Exec()
			arg_list = [str(self.translator_executable_path), str(parsed_path), str(DTS_path)]
			execTranslate.exec_list(arg_list)
			stdout, stderr = execTranslate.get_output()
			
			if stdout.find('Duration:') == -1:
				print('\n\n--Translator Failure!!')

				# when failure delete translated folder if still there
				self.remove_translated(translated_path)
				self.failure_stats['translator_failures'].append(ch10path)
				continue


			if not os.path.isdir(translated_path):
				print('\n\n--Translator Failure!!\nexpected translator output: {}'.format(translated_path))
				self.failure_stats['translator_failures'].append(ch10path)
				continue

			high_level_metadata['Translated'] = True

			# write high level metadata (overwrite with Translated = True)
			high_level_metadata_yaml_path = os.path.join(chapter10_database_folder_path,'_high_level_metadata.yaml');
			with open(high_level_metadata_yaml_path, 'w') as file:
				print('\n--Writing _high_level_metadata: \n{}\n'.format(high_level_metadata_yaml_path))
				yaml.dump(high_level_metadata,file)

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
	'''

	def write_failure_stats(self):
		with open(self.failure_yaml_path, 'w') as file:
			print('--Writing failure stats to yaml file: \n{}'.format(self.failure_yaml_path))
			yaml.dump(self.failure_stats,file)

	def delete_chapter10_database_folder(self, ch10_database_folder, chapter10name):
		parsed_path = os.path.join(ch10_database_folder, str(chapter10name) + '_1553.parquet')
		ethernet_path = os.path.join(ch10_database_folder, str(chapter10name) + '_ethernet.parquet')
		translated_path = os.path.join(ch10_database_folder, str(chapter10name) + '_1553_translated')

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

		# remove ch10_database_folder if present
		if os.path.isdir(ch10_database_folder):
			print('Deleting ch10_database_folder directory \n{}\n'.format(ch10_database_folder))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  ch10_database_folder
			shutil.rmtree(ch10_database_folder)

	def remove_translated(self, translated_path):

		# remove translated if exists
		if os.path.isdir(translated_path):
			print('Deleting translated directory \n{}\n'.format(translated_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  translated_path
			shutil.rmtree(delete_path)


	def exec(self):		

		self.initialize_paths()

		self.parse_and_translate()

		self.write_failure_stats()		