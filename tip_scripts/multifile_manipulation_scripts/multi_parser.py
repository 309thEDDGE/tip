from xml.dom import minidom
import subprocess
import time
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
	def __init__(self, config_data, tip_root_path, datafiles_path, dts_database_path, output_path):
		self.config_data = config_data
		self.datafiles_path = datafiles_path
		self.dts_database_path = dts_database_path
		self.output_path = output_path
		self.tip_root_path = tip_root_path
		self.parser_executable_path = Path("")
		self.translator_executable_path = Path("")
		self.combined_metadata = []
		self.multi_parse_stats = {'parser_failures': [], 
									'translator_failures': [], 
									'duplicate_ch10_file_paths': {},
									'non_overwrite_skips': [],
									'DTS_config_not_found': [],
									'exclusion_skips': []}


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
		multi_parse_stats_dir = os.path.join(self.output_path,'multi_parse_stats')
		if not os.path.isdir(multi_parse_stats_dir):
			print('creating directory {}'.format(multi_parse_stats_dir))
			os.mkdir(multi_parse_stats_dir)


		# append time stamp to file name for failure stats yaml file
		now = datetime.now()
		# dd/mm/YY H:M:S
		dt_string = now.strftime("%m%d%Y_%H_%M_%S")
		# write failure statistics to yaml
		self.multi_parse_stats_yaml_path = os.path.join(multi_parse_stats_dir,'_' + dt_string + '_multi_parse_stats.yaml');

		# update the working directly to bin for the executable to find
		# config files
		os.chdir(os.path.join(self.tip_root_path,'bin'))

	def return_DTS_path(self, DTS_config):
		DTS_path_yaml = os.path.join(self.dts_database_path, DTS_config + '_ICD.yaml')
		DTS_path_txt = os.path.join(self.dts_database_path, DTS_config + '_ICD.txt')
		DTS_path = Path('')
		if os.path.isfile(DTS_path_yaml):
			return DTS_path_yaml

		elif os.path.isfile(DTS_path_txt):
			return DTS_path_txt

		else:
			return 'NONE'

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

			# Skip based off exclusion and inclusion config restrictions
			all_caps_ch10_path = ch10path.upper()

			exclude = False

			# if exclusion string is a substring of the 
			# chapter 10 path, do not process the 
			# chapter 10. Case insensitive
			for exclusion in self.config_data['exclusion_path_substrings']:
				exclusion = exclusion.upper()

				if all_caps_ch10_path.find(exclusion) != -1:
					exclude = True
					print('exclusion string {} exists in chapter 10 path'
							', skipping chapter 10 {}'.format(exclusion, ch10path))

			# if inclusion string is not a substring of the 
			# chapter 10 path, do not process the 
			# chapter 10. Case insensitive
			for inclusion in self.config_data['inclusion_path_substrings']:
				inclusion = inclusion.upper()

				if all_caps_ch10_path.find(inclusion) == -1:
					exclude = True
					print('inclusion string {} does not exist in chapter 10 path'
							', skipping chapter 10 {}'.format(inclusion, ch10path))

			if exclude:
				self.multi_parse_stats['exclusion_skips'].append(ch10path)
				continue


			# If the chapter 10 name has already been 
			# processed, add it to the failure statistics
			# and continue.
			if chapter10name in ch10_names.keys():
				if chapter10name not in self.multi_parse_stats['duplicate_ch10_file_paths'].keys():
					self.multi_parse_stats['duplicate_ch10_file_paths'][chapter10name] = []
					# make sure to insert the first chatper 10 path associated with the first chatper 10 found 
					self.multi_parse_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10_names[chapter10name])
					self.multi_parse_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10path)
				else:
					self.multi_parse_stats['duplicate_ch10_file_paths'][chapter10name].append(ch10path)
				continue

			ch10_names[chapter10name] = ch10path

			# Check for valid DTS config
			DTS_identifier_method = 'NONE'
			DTS_path = ''

			XML_identification = 'NONE'
			path_identification = 'NONE'
			default_identification = 'NONE'

			XML_identification = self.return_DTS_path(datafile.attributes['AircraftCometName'].value)

			# check folders in the path
			subfolders = self.splitall(ch10path)
			path_config = 'NONE'
			found = False
			for k, v in self.config_data['DTS_config_path_maps'].items():
				for subfolder in subfolders:
					# convert to upper case for case insensitivity
					if subfolder.upper() == k.upper():
						path_config = v
						found = True
						break
				if found:
					path_identification = self.return_DTS_path(path_config)
					break

			default_identification = self.return_DTS_path(self.config_data['default_DTS_config'])

			if XML_identification != 'NONE':
				DTS_identifier_method = 'XML_identification'
				DTS_path = XML_identification
				DTS_config = datafile.attributes['AircraftCometName'].value
			elif path_identification != 'NONE':
				DTS_identifier_method = 'path_identification'
				DTS_path = path_identification
				DTS_config = path_config
			elif default_identification != 'NONE':
				DTS_identifier_method = 'default_identification'
				DTS_path = default_identification
				DTS_config = self.config_data['default_DTS_config']
			else:
				self.multi_parse_stats['DTS_config_not_found'].append(ch10path)
				DTS_config = 'NONE'
				print('DTS file not found!')

			# Check if chapter 10 folder exists
			if os.path.isdir(chapter10_database_folder_path):
				# if overwrite = True, delete any old data
				if self.config_data['overwrite']:
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

					# Translation and parser data exist
					else:
						# check if metadata exists
						metadata_path = os.path.join(translated_path,'_metadata.yaml')
						if os.path.isfile(metadata_path):

							file_data = {}
							with open(metadata_path) as file:
								file_data = yaml.load(file)
	
							# check if dts path was logged
							if 'dts_path' in file_data.keys():
								# if dts path is different re translate
								if file_data['dts_path'] == DTS_path:
									print('Data already exists for {}, skipping parse and translate steps'.format(chapter10name))
									print(DTS_path)
									self.multi_parse_stats['non_overwrite_skips'].append(ch10path)
									continue
								else:
									execute_translator_only = True
									self.remove_translated(translated_path)
									print('DTS path updated, re-execution translation'.format(metadata_path))

							else:
								execute_translator_only = True
								self.remove_translated(translated_path)
								print('Translator metadata missing dts_path {}, re-executing translation'.format(metadata_path))

						else:
							execute_translator_only = True
							self.remove_translated(translated_path)
							print('Translator metadata does not exist for {}, re-executing translation'.format(chapter10name))
						

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
									'DTS_identifier_method': DTS_identifier_method,
									'Pilot': datafile.attributes['Pilot'].value, 
									'CallSign': datafile.attributes['CallSign'].value
									}

			high_level_metadata_yaml_path = os.path.join(chapter10_database_folder_path,'_high_level_metadata.yaml');
			with open(high_level_metadata_yaml_path, 'w') as file:
				print('\n--Writing _high_level_metadata: \n{}\n'.format(high_level_metadata_yaml_path))
				yaml.dump(high_level_metadata,file)			


			print('---------chapter10 name: {}---------\n'.format(tail))

			if not execute_translator_only:					

				# check if the file is a chapter 10 file
				if ext != '.ch10':
					print('\n\n--Parser Failure!!\nfilename {} is not a chapter 10'.format(ch10path))
					self.multi_parse_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				# check if the chapter 10 exists
				if not os.path.isfile(ch10path):
					print('\n\n--Parser Failure!!\nch10 path {} does not exist'.format(ch10path))
					self.multi_parse_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				print('---Parse--- \n{}'.format(ch10path))

				execParse = Exec()
				execParse.exec_list([str(self.parser_executable_path), str(ch10path), chapter10_database_folder_path])
				stdout, stderr = execParse.get_output()				
				print('Parsed Path: {}'.format(parsed_path))
				if stdout.find('Duration:') == -1:
					print('\n\n--Parser Failure!!')
					self.multi_parse_stats['parser_failures'].append(ch10path)
					self.delete_chapter10_database_folder(chapter10_database_folder_path, chapter10name)
					continue

				if not os.path.isdir(parsed_path):
					print('\n\n--Parser Failure!!\nexpected parser output: {}'.format(parsed_path))
					self.multi_parse_stats['parser_failures'].append(ch10path)
					continue
					
				print('SUCCESS')
				high_level_metadata['Parsed'] = True


				print('Parser Success:\n {}\n\n'.format(parsed_path))


			# start translation routine
			print('---Translate--- \n{}'.format(parsed_path))

			# check if parsed file exists
			if not os.path.isdir(parsed_path):
				print('\n\n--Translator Failure!!\nparsed file {} does not exist'.format(parsed_path))
				self.remove_translated(translated_path)
				self.multi_parse_stats['translator_failures'].append(ch10path)
				continue

			if not os.path.isfile(DTS_path):
				self.remove_translated(translated_path)
				# The reason for not adding to multi_parse_stats['translator_failures']
				# is because this case should have been added previously to 
				# multi_parse_stats['DTS_config_not_found']
				continue
			

			execTranslate = Exec()
			arg_list = [str(self.translator_executable_path), str(parsed_path), str(DTS_path)]
			execTranslate.exec_list(arg_list)
			stdout, stderr = execTranslate.get_output()
			
			if stdout.find('Duration:') == -1:
				print('\n\n--Translator Failure!!')

				# when failure delete translated folder if still there
				self.remove_translated(translated_path)
				self.multi_parse_stats['translator_failures'].append(ch10path)
				continue


			if not os.path.isdir(translated_path):
				print('\n\n--Translator Failure!!\nexpected translator output: {}'.format(translated_path))
				self.multi_parse_stats['translator_failures'].append(ch10path)
				continue

			print('Translator Success:\n {}\n'.format(translated_path))


	# taken from https://www.oreilly.com/library/view/python-cookbook/0596001673/ch04s16.html
	def splitall(self, path):
		allparts = []
		while 1:
			parts = os.path.split(path)
			if parts[0] == path:  # sentinel for absolute paths
				allparts.insert(0, parts[0])
				break
			elif parts[1] == path: # sentinel for relative paths
				allparts.insert(0, parts[1])
				break
			else:
				path = parts[0]
				allparts.insert(0, parts[1])
		return allparts
		

	def write_failure_stats(self):
		with open(self.multi_parse_stats_yaml_path, 'w') as file:
			print('--Writing failure stats to yaml file: \n{}'.format(self.multi_parse_stats_yaml_path))
			yaml.dump(self.multi_parse_stats,file)

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

		time.sleep(.03)

	def remove_translated(self, translated_path):

		# remove translated if exists
		if os.path.isdir(translated_path):
			print('Deleting translated directory \n{}\n'.format(translated_path))
			# Must prepend to path in case path is > 260 characters
			delete_path = '\\\\?\\' +  translated_path
			shutil.rmtree(delete_path)

		time.sleep(.03)


	def exec(self):		

		self.initialize_paths()

		self.parse_and_translate()

		self.write_failure_stats()		