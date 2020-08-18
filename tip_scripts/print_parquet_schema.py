from pyspark.sql import SparkSession
import sys
spark = SparkSession.builder \
    .master('local') \
    .appName('test') \
    .config('spark.executor.memory', '6g') \
    .config("spark.cores.max", "4") \
    .getOrCreate()

infile = sys.argv[1]
df = spark.read.parquet(infile)
#df = df.filter(df.name == 'NONE')
df.printSchema()
df = df.select('time', 'blah')
df.show(20)
#df.describe().show()
print('count', df.count())
print('n cols', len(df.columns))
spark.stop()