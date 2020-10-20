#!/usr/bin/python3

import os
import sys
import yaml

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print('run_tip.py: not enough args!')
        sys.exit(0)

    raw_yaml = sys.argv[1]
    data = yaml.load(raw_yaml, Loader=yaml.FullLoader)
    print('loaded data:\n', data)
    sys.exit(0)
