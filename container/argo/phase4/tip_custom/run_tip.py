#!/usr/bin/python3

import os
import sys
import json

if __name__ == '__main__':

    debug = True
    if len(sys.argv) < 2:
        print('run_tip.py: not enough args!')
        sys.exit(0)

    raw_json = sys.argv[1]
    #data = yaml.load(raw_yaml, Loader=yaml.FullLoader)
    data = json.loads(raw_json)

    if debug:
        print('loaded data:\n', data)

    ch10mntpath = '/ch10mnt'
    icdmntpath = '/icdmnt'
    
    if debug:
        

        lsch10 = ''
        lsicd = ''
        try:
            lsch10 = os.listdir(ch10mntpath)
        except FileNotFoundError as e:
            print(e)

        try:
            lsicd = os.listdir(icdmntpath)
        except FileNotFoundError as e:
            print(e)

        print('\nls ch10 mount path:\n', lsch10)
        print('\nls icd mount path:\n', lsicd)

    # Construct call to parse_and_translate.py
    script_path = '/usr/local/tip/parse_and_translate.py'
    ch10_full_path = os.path.join(ch10mntpath, data['ch10_file_name'])
    icd_full_path = os.path.join(icdmntpath, data['icd_file_name'])
    sys_call = 'python3 {:s} {:s} {:s}'.format(script_path, ch10_full_path, icd_full_path)
    print('sys_call = {:s}'.format(sys_call))
    os.system(sys_call)
        
    sys.exit(0)
