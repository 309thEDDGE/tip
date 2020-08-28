import sys
from pyspark.sql import SparkSession
from pyspark.sql.functions import col
import numpy as np

# For isnan and isinf
import math

spark = SparkSession.builder.appName("Python Spark DataFrame Example").config("spark.some.config.option","value").getOrCreate()
#read the data in from the command line
df1 = spark.read.parquet(sys.argv[1])
df2 = spark.read.parquet(sys.argv[2])

if len(sys.argv) > 3:
	if sys.argv[3] == '--sort':
		print('\nSorting ...')
		df1 = df1.orderBy('time')
		df2 = df2.orderBy('time')
	elif sys.argv[3] == '--no-sort':
		print('\nNot sorting ... ')

strict_cols = True
if len(sys.argv) > 4:
	if sys.argv[4] == '--no-col-count':
		print("\nNot adhering to strict column count. Comparing input 2 columns only.")
		strict_cols = False
	elif sys.argv[4] == '--with-col-count':
		strict_cols = True
		
req_same_row_count = True
if len(sys.argv) > 5:
	if sys.argv[5] == '--no-row-count':
		print("\nNot adhering to strict row count.")
		req_same_row_count = False
	elif sys.argv[5] == '--with-row-count':
		req_same_row_count = True

skip_col_names = []

#before we can do any manipulations, we need to make sure the files are equal in both columns and rows
#check to see if the row count matches if not then it is not the same
count1 = df1.count()
count2 = df2.count()
row_count_equal = False
if count1 == count2:
	row_count_equal = True
print('count1 = {:d}, count2 = {:d}'.format(count1, count2))
if row_count_equal or not req_same_row_count:
	#check to see if the column count matches if not then it is not the same
	if strict_cols:
		if len(df1.columns) == len(df2.columns):
		#If we get in here then we know we can compare the two files
		#do the individual column comparison code so we can see if they're the same or not.
			#put them both into Schemas
			schema1 = df1.schema
			schema2 = df2.schema
			stats = {}
			#make a couple of new lines for easy reading
			print("\n")
			#we need a for loop to iterate through the whole thing
			#a for loop that starts at zero and goes til the end of the column length
			for column in range (0, len(df1.columns)):
				if column in skip_col_names:
					print('Skipping column: {:s}'.format(column))
					continue
				#put get the values of the first column of both files into lists
				df1_col1 = df1.select(schema1[column].name)
				native_dictionary1 = df1_col1.collect()
				df2_col1 = df2.select(schema2[column].name)
				native_dictionary2 = df2_col1.collect()	
				stats[schema1[column].name] = 0
				print("Comparing " + schema1[column].name + " column in File 1 with " + schema2[column].name +" column in File 2.")
				print()
				#Iterate through the first dictionary and compare the values of both columns
				for row in range(len(native_dictionary1)):	
					if native_dictionary1[row][schema1[column].name] != native_dictionary2[row][schema2[column].name]:
						print("------------------------------------------------------------------------")
						print("MISMATCH FOUND! SEE LINE BELOW FOR DETAILS ON WHERE THE MISMATCH OCCURED.")
						print("On row {} The following two values did not match {} and {}.".format(row, native_dictionary1[row][schema1[column].name], native_dictionary2[row][schema2[column].name]))
						print("------------------------------------------------------------------------")
						print("Now continuing to compare all other columns.")
						print("\n")
						stats[schema1[column].name] += 1
				native_dictionary1.clear()
				native_dictionary2.clear()
			print("\n\n-----Comparison Results-----")
			for key in stats:
				print("Column {} Had {} Mismatches".format(key, stats[key]))
			print("------End Of Results-------")
			print("\n\n")
		#If we get here then the columns must not be equal
		else:
			print("\n\n")
			print("------------------------------------------------------------------------")
			print("MISMATCH FOUND! SEE LINES BELOW FOR DETAILS ON WHERE THE MISMATCH OCCURED.")
			print("The following two parquet files do NOT match:", sys.argv[1], sys.argv[2])
			print("The files listed above do not conatin the same amount of Columns.")
			print("------------------------------------------------------------------------")
	# If here then column count is not required to be equal.
	else:
		schema1 = df1.schema
		schema2 = df2.schema
		schema1_field_names = schema1.fieldNames()
		stats = {}
		#make a couple of new lines for easy reading
		print("\n")
		#we need a for loop to iterate through the whole thing
		#a for loop that starts at zero and goes til the end of the column length
		time_col_data = None
		is_time_col = False
		have_time_col = False
		for column in df2.columns:
			
			##### Debug #####
			#if column != 'data':
			#	continue

			#if schema2[column].simpleString().split(':')[1] == 'boolean':
			#	print('Skipping boolean column {:s}'.format(column))
			#	continue

			##### end Debug #####
			
			if column in schema1_field_names:
				if column in skip_col_names:
					print('Skipping column: {:s}'.format(column))
					continue
				is_time_col = False
				df1_col1 = df1.select(column)
				native_dictionary1 = df1_col1.collect()
				df2_col1 = df2.select(column)
				native_dictionary2 = df2_col1.collect()	
				stats[column] = 0
				col1count = len(native_dictionary1)
				col2count = len(native_dictionary2)
				use_count = col1count
				if not req_same_row_count and not row_count_equal:
					if col2count < col1count:
						print('Using count from column 2 ({:d}) as the row count!'.format(col2count))
						use_count = col2count
				print("Comparing " + column + " column in File 1 with " + column +" column in File 2.\n")
				
				#Iterate through the first dictionary and compare the values of both columns
				if column == 'time':
					print('found time col!')
					is_time_col = True
					have_time_col = True
					time_col_data = np.zeros(use_count, dtype='uint64')
				for row in range(use_count):
					
					#if column == 'CI03_I-2509':
					#	print('val1 = {}, val2 = {}'.format(native_dictionary1[row][column], native_dictionary2[row][column]))
					
					# Check if col contains a list.
					if isinstance(native_dictionary1[row][column], list):
						dlist1 = native_dictionary1[row][column]
						dlist2 = native_dictionary2[row][column]
						if len(dlist1) != len(dlist2):
							print('Length of list1 ({:d}) not equal to length of list2 ({:d})'.format(
								len(dlist1), len(dlist2)))
							continue
						
						for i in range(len(dlist1)):
							if dlist1[i] != dlist2[i]:
								if have_time_col:
									print("time {:d}, row {}: no match {}, {}.".format(time_col_data[row],
										row, dlist1, dlist2))
								else:
									print("row {}: lists don't match {} and {}.".format(
										row, dlist1, dlist2))
								stats[column] += 1
								break
					else:
						if is_time_col:
							time_col_data[row] = native_dictionary1[row][column]
						# Check for Nan and (-)inf.
						if math.isnan(native_dictionary1[row][column]) or math.isinf(native_dictionary1[row][column]):
							continue

						if native_dictionary1[row][column] != native_dictionary2[row][column]:
							if have_time_col:
								print("(time {:d}, row {}: no match ({}, {}).".format(time_col_data[row], row, native_dictionary1[row][column], native_dictionary2[row][column]))
							else:
								print("On row {} The following two values did not match {} and {}.".format(row, native_dictionary1[row][column], native_dictionary2[row][column]))
							stats[column] += 1
							
				native_dictionary1.clear()
				native_dictionary2.clear()
				
		print("\n\n-----Comparison Results-----")
		for key in stats:
			print("Column {} Had {} Mismatches".format(key, stats[key]))
		print("------End Of Results-------")
		print("\n\n")
#If we get here then the rows most not be equal
else: 
	print("\n\n")
	print("------------------------------------------------------------------------")
	print("MISMATCH FOUND! SEE LINES BELOW FOR DETAILS ON WHERE THE MISMATCH OCCURED.")
	print("The following two parquet files do NOT match:", sys.argv[1], sys.argv[2])
	print("The files listed above do not conatin the same amount of Rows.")
	print("------------------------------------------------------------------------")