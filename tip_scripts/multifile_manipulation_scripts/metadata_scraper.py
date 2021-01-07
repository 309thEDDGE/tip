import yaml
import os
import sys

class MetadataScraper:
	def __init__(self, database_path):
		self.database_path = database_path
		self.parquet_data_path = os.path.join(database_path, 'parquet_data')

	def exec(self):

		# Check for parquet data
		if not os.path.isdir(self.parquet_data_path):
			sys.exit('parquet data path does not exist {}'.format(self.parquet_data_path))

		chapter10_folders = os.listdir(self.parquet_data_path)

		combined_metadata = []

		for chapter10_folder in chapter10_folders:
			ch10path = os.path.join(self.parquet_data_path, chapter10_folder)
			metadata_path = os.path.join(ch10path, '_high_level_metadata.yaml')

			# check for high level metadata
			if not os.path.isfile(metadata_path):
				print('metadata path missing {}, skipping ch10\n'.format(metadata_path))
				continue

			file_data = {}

			with open(metadata_path) as file:
				file_data = yaml.load(file)

			file_data['Parsed'] = False
			file_data['Translated'] = False
			file_data['Full_Busmap'] = False
			file_data['DTS_config'] = 'NONE'

			# check for 1553 parquet metadata
			parser_metadata_path = os.path.join(file_data['1553_parquet_path'],'_metadata.yaml')
			if not os.path.isfile(parser_metadata_path):
				print('1553 parser metadata missing {}, skipping ch10\n'.format(parser_metadata_path))
				continue

			file_data['Parsed'] = True

			# check for translator metadata
			translator_metadata_path = os.path.join(file_data['1553_translated_path'],'_metadata.yaml')
			if not os.path.isfile(translator_metadata_path):
				print('1553 translator metadata missing {}\n'.format(parser_metadata_path))

			else:
				translator_metadata = {}
				with open(translator_metadata_path) as file:
					translator_metadata = yaml.load(file)

				# check if valid metadata file
				if 'dts_path' in translator_metadata and 'excluded_channel_ids' in translator_metadata:
					dts_path = translator_metadata['dts_path']
					head, tail = os.path.split(dts_path)
					dts_config , ext = os.path.splitext(tail)
					file_data['DTS_config'] = dts_config
					if len(translator_metadata['excluded_channel_ids']) == 0:
						file_data['Full_Busmap'] = True

					file_data['Translated'] = True


			combined_metadata.append(file_data)


		# write metadata
		combined_yaml_path = os.path.join(self.database_path,'_combined_metadata.yaml');
		with open(combined_yaml_path, 'w') as file:
			print('\n--Writing combined yaml meta_data: \n{}\n\n'.format(combined_yaml_path))
			yaml.dump(combined_metadata, file)