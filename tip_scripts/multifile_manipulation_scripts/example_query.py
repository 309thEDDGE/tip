import yaml
import os
from pathlib import Path

base_path = Path(r'C:\Users\A10\Documents\Source\locker\mutli_file_manipulation\multi_files')

################ metadata search  ################
yaml_path = os.path.join(base_path, '_combined_metadata.yaml')
file_data = []

with open(yaml_path) as file:
    file_data = yaml.full_load(file)

translated_paths = []

message_name = 'MessageName'

for metadata in file_data:
	# check if the chapter 10 has translation data
	if metadata['Translated']:
		# check if the translated data has a specific message
		file_path = os.path.join(metadata['1553_translated_path'],metadata['ch10name'] + '_1553_translated_' + message_name + '.parquet')
		# if the message exists add it to list of paths needed for later analysis
		if os.path.isdir(file_path):
			translated_paths.append(str(file_path))



################ pyspark query ################
from pyspark.sql import SparkSession
import pyspark.sql.functions as f
spark = SparkSession.builder.appName("Python Spark DataFrame Example").config("spark.some.config.option","value").getOrCreate()

ch10_matches = []

print(translated_paths)

for path in translated_paths:
	df = spark.read.parquet(path)

	# Check if a column is in a specific range
	df_alt_filter = df.filter((f.col('MessageName-1') < 2230) & (f.col('MessageName-1') > 2226))
	if df_alt_filter.count() > 1:
		ch10_matches.append(path)

print('--Results\n')
print('\n\n\n\n\n--- {} match(es)---'.format(len(ch10_matches)))
for ch10 in ch10_matches:
	head, tail = os.path.split(ch10)
	print('-> {}'.format(tail))

print('---\n\n')




