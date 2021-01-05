import os, sys
import ctypes
import glob
import tip_parse

def parse(input_path, output_path):

    res = None

    tip_root_path = os.path.dirname(os.path.abspath(
        os.path.join(os.path.realpath(__file__), '../../..')))

    res = tip_parse.run_parser(input_path, output_path, tip_root_path)

    return res

if __name__ == '__main__':

    res = parse(sys.argv[1], sys.argv[2])
    print('res:', res)