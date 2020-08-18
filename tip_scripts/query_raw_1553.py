from pyspark.sql import SparkSession
import sys, os
import numpy as np

spark = SparkSession.builder \
    .master('local') \
    .appName('test') \
    .config('spark.executor.memory', '6g') \
    .config("spark.cores.max", "4") \
    .getOrCreate()

# Import digital ICD as dataframe.
path = sys.argv[1]
df = spark.read.parquet(path)

df = df.drop('time', 'doy', 'data').filter((df.channelid == 33) & (df.rxcommwrd == 39030))
df.show()

spark.stop()