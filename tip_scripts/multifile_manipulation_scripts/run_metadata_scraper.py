import os
import sys

tip_root_path = os.path.dirname(os.path.abspath(os.path.join(os.path.realpath(__file__), '../..')))
sys.path.append(tip_root_path)

from tip_scripts.multifile_manipulation_scripts.metadata_scraper import MetadataScraper

# Argument parser
import argparse


if __name__ == '__main__':

	#
	# Setup command line arguments
	#
	aparse = argparse.ArgumentParser(description='Generate metadata from parquet database.')
	aparse.add_argument('database_path', metavar='<database path>', type=str, 
						help='Path to database. Should include folders multi_parse_stats and parquet_data')

	args = aparse.parse_args()

	scraper = MetadataScraper(args.database_path)

	scraper.exec()