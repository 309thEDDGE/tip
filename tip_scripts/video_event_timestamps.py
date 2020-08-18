from pyspark.sql import SparkSession
from pyspark.sql.functions import col
import parquet_tools
import sys, os
import numpy as np

max_part_bytes = 1024 * 1024 * 512
spark = SparkSession.builder \
	.master('local') \
	.appName('show') \
	.config('spark.driver.memory', '10g') \
	.config("spark.cores.max", "8") \
	.config('spark.driver.host', 'localhost') \
	.config('spark.driver.bindAddress', '127.0.0.1') \
	.config('spark.debug.maxToStringFields', '200') \
	.config('spark.driver.maxResultSize', '4g') \
	.config('spark.files.maxPartitionBytes', str(max_part_bytes)) \
	.getOrCreate()

# TD = Translated Data
td_dir = r'path'
#pq_fname = sys.argv[1]

msg_name = 'Message_name'
wordno = 1
df = parquet_tools.get_translated_message_df(spark, td_dir, msg_name)
if df is None:
	sys.exit(0)
elem_substr = parquet_tools.get_element_name(msg_name, wordno)
cols = parquet_tools.get_valid_cols_matching_substr(df.schema, elem_substr)
cols.append('time')
df_subset = df.select(cols)
#df_subset = df_subset.groupBy().max().show()
#df_subset.describe().show()

flag_to_bitmsb_dict = {'message': 13, 'warning': 14, 'caution': 15, 'note': 16}
for flag_name in flag_to_bitmsb_dict.keys():
	elem_name = parquet_tools.get_element_name(msg_name, 1, bitmsb=flag_to_bitmsb_dict[flag_name])
	print('\n{:s}:'.format(elem_name))

	tempdf = df_subset.filter(col(elem_name) == 1).orderBy('time')
	print(tempdf.count())

	# Get time and data.
	rows = tempdf.collect()
	N = len(rows)
	time_data = np.array([r['time'] for r in rows], dtype='uint64')
	col_data = np.array([r[elem_name] for r in rows], dtype='uint8')

	# output file name
	bname = '{:s}_{:s}_{:s}.txt'.format(os.path.basename(td_dir), elem_name, flag_name)
	print(bname)
	with open(os.path.join(td_dir, bname), 'w') as f:
		f.write('Epoch Time [ns], {:s}\n'.format(elem_name))
		for i in range(N):
			f.write('{:020d}, {:01d}\n'.format(time_data[i], col_data[i]))

spark.stop()