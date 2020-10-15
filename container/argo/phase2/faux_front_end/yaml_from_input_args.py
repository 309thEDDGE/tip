#!/usr/local/bin/python3

import os
import sys
import time
import yaml
import signal

# Stolen from: https://stackoverflow.com/questions/18499497/how-to-process-sigterm-signal-gracefully
class GracefulKiller:
    kill_now = False
    signum = None
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_gracefully)
        signal.signal(signal.SIGTERM, self.exit_gracefully)

    def exit_gracefully(self,signum, frame):
        self.kill_now = True
        self.signum = signum

if __name__ == '__main__':

    # Create file for liveness probe.
    os.system('touch /app/healthy')

    # Wait for system to check.
    time.sleep(10)

    # Create file for readiness probe.
    os.system('touch /app/ready')
    
    # Setup dict for yaml file.
    yamloutput = {
        'status': None,
        'ext_mnt_path': None,
        'another': 2344
        }

    req_arg_count = 2
    if(len(sys.argv) < req_arg_count):
        print('Not enough args!')
        yamloutput['status'] = 'FAIL'
        os.system('echo FAIL > /app/termination-log')
        sys.exit(0)

    out_file_name = 'data.yml'
    out_path = os.path.join('/app', out_file_name)

    # Set output values.
    yamloutput['ext_mnt_path'] = sys.argv[1]

    # Set good status
    yamloutput['status'] = 'OK'

    # Write yaml file.
    with open(out_path, 'w') as f:
        yaml.dump(yamloutput, f)
    print(yaml.dump(yamloutput) + '\n')

    reps = 30
    wait = 1
    ind = 0
    killer = GracefulKiller()
    while ind < reps:
        if killer.kill_now:
            print('Caught signal: {:d}'.format(killer.signum))
            break
        #print('waiting ....')
        time.sleep(wait)
        ind += 1

    # debug
    os.system('cat {:s}'.format(out_path))

    # termination message
    os.system('echo OK > /app/termination-log')

    # not ready
    os.system('rm /app/ready')

    time.sleep(5)

    # Remove liveness probe check.
    os.system('rm /app/healthy')
    sys.exit(0)
