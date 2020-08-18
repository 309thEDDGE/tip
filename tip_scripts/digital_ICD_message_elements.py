from pyspark.sql import SparkSession
import sys, os
from import_digital_ICD_into_spark_dataframe import *
import numpy as np

spark = SparkSession.builder \
    .master('local') \
    .appName('test') \
    .config('spark.executor.memory', '6g') \
    .config("spark.cores.max", "4") \
    .getOrCreate()

# Import digital ICD as dataframe.
icd_path = sys.argv[1]
icd_df = import_digital_icd(spark, icd_path)

rows = icd_df.filter(icd_df.msg_name == "MSG_NAME").collect()
names = [r['elem_name'] for r in rows]

name_str = '{ '
for i, name in enumerate(names):
    name_str += '\"{:s}\", '.format(name)
name_str += '}'
print(name_str)
spark.stop()