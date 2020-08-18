from pyspark.sql import SparkSession
import sys
import matplotlib.pyplot as plt
import numpy as np
spark = SparkSession.builder \
    .master('local') \
    .appName('test') \
    .config('spark.executor.memory', '6g') \
    .config("spark.cores.max", "4") \
    .getOrCreate()

infile = sys.argv[1]
word_ind = int(sys.argv[2])
if word_ind < 0 or word_ind > 31:
    print('word_ind invalid: {:d}'.format(word_ind))

df = spark.read.parquet(infile).select('time', 'data')
collect_data = df.collect()
time_data = np.array([row['time'] for row in collect_data], dtype='uint64')
word_data = np.array([row['data'][word_ind] for row in collect_data], dtype='uint16')

time_data = np.subtract(time_data, np.min(time_data)) / int(1e9)


plt.figure(figsize=(8,6), dpi=150, facecolor='white')
plt.scatter(time_data, word_data)

spark.stop()