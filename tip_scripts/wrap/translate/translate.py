import os, sys
import tip_translate

wrap_util_path = os.path.dirname(os.path.abspath(os.path.join(
        os.path.realpath(__file__), '..')))
sys.path.append(wrap_util_path)

from wrap_util.wrap_util import *

def translate(input_path, dts_path):

    res = None

    tip_root_path = get_root_path()

    res = tip_translate.run_translator(input_path, dts_path, tip_root_path)

    if res is None:
        print('tip_translate.run_translator: RunTranslator was not called, likely due to malformed args.')

    return res

if __name__ == '__main__':

    res = translate(sys.argv[1], sys.argv[2])
    print('res:', res)