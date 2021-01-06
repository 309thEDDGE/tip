import os, sys
from pathlib import Path
import tip_parse

wrap_util_path = os.path.dirname(os.path.abspath(os.path.join(
        os.path.realpath(__file__), '..')))
sys.path.append(wrap_util_path)

from wrap_util.wrap_util import *

def parse(input_path, output_path):

    res = None

    tip_root_path = get_root_path()

    if output_path is None:
        output_path = str(Path(input_path).parent)
        print('output_path:', output_path)

    res = tip_parse.run_parser(input_path, output_path, tip_root_path)

    if res is None:
        print('tip_parse.run_parser: Call to RunParser was not made, likely due to malformed args.')

    return res

if __name__ == '__main__':

    if len(sys.argv) == 2:
        res = parse(sys.argv[1], None)
    elif len(sys.argv) == 3:
         res = parse(sys.argv[1], sys.argv[2])
    print('res:', res)