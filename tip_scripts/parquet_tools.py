
import os, sys

def get_translated_message_df(spark, td_dir, msg_name):
	tmname = '{:s}_{:s}.parquet'.format(os.path.basename(td_dir), msg_name)
	tmpath = os.path.join(td_dir, tmname)
	print(tmpath)

	if os.path.exists(tmpath):
		return spark.read.parquet(tmpath)
	else:
		return None

def get_element_name(msg_name, wordno, bitmsb=None):
	elem_name = '{:s}-{:02d}'.format(msg_name, wordno)
	if bitmsb is not None:
		elem_name += '{:02d}'.format(bitmsb)
	return elem_name

def get_valid_cols_matching_substr(schema, str):
	valid_cols = []
	for elem in schema:
		if elem.name.find(str) > -1:
			valid_cols.append(elem.name)

	return valid_cols

