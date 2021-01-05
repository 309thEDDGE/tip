import os, sys
import tip_parse

wrap_util_path = os.path.dirname(os.path.abspath(os.path.join(
        os.path.realpath(__file__), '..')))
sys.path.append(wrap_util_path)

from wrap_util.wrap_util import *

def parse(input_path, output_path):

    res = None

    tip_root_path = get_root_path()

    res = tip_parse.run_parser(input_path, output_path, tip_root_path)

    return res

if __name__ == '__main__':

    res = parse(sys.argv[1], sys.argv[2])
    print('res:', res)