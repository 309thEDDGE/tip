#importing necessary libraries
import sys
from pyspark.sql import SparkSession
from pyspark.sql.functions import col
spark = SparkSession.builder.appName("Python Spark DataFrame Example").config("spark.some.config.option","value").getOrCreate()
#Read the data in from the command line
df1 = spark.read.parquet(sys.argv[1])
df2 = spark.read.parquet(sys.argv[2])
#before we can do any manipulations, we need to make sure the files are equal in both columns and rows
#check to see if the row count matches if not then it is not the same
if df1.count() == df2.count():
	#check to see if the column count matches if not then it is not the same
	if len(df1.columns) == len(df2.columns):
	#If we get in here then we know we can compare the two files
	#do the actual manipulations so we can find out what is different between the two files
		df3 = df1.subtract(df2)
		df4 = df2.subtract(df1)
	#check to see if the are the same or not and print out the results
	#In order for them to match, they both must equal zero.
		if df3.count() == 0 & df4.count() == 0:
			print()
			print()
			print("The following two parquet files match --->", sys.argv[1], sys.argv[2])
		else:
			print()
			print()
			print("The following two parquet files do NOT match --->", sys.argv[1], sys.argv[2])
	#If we get here then the columns must not be equal
	else:
		print()
		print()
		print("The following two parquet files do NOT match --->", sys.argv[1], sys.argv[2])
		print("Columns are not the same amount")
#If we get here then the rows most not be equal
else: 
	print()
	print()
	print("The following two parquet files do NOT match --->", sys.argv[1], sys.argv[2])
	print("Rows are not the same amount")