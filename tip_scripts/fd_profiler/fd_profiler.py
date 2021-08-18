from datetime import datetime
import psutil
import subprocess
import time
import os, sys

def main(proc_name, poll_period_sec):
    
    proc_obj = wait_for_process(proc_name)
    print(f"Process \'{proc_obj['name']}\' has PID = {proc_obj['pid']}")

    open_fds, rlimit_nofile, previous_rlimit_nofile = None, None, None
    pid = proc_obj['pid']
    while True:
        open_fds, rlimit_nofile = pid_open_fds(pid)

        if open_fds is None:
            print(f"RLIMIT_NOFILE = (soft, hard) {previous_rlimit_nofile}")
            break
        else:
            time_str = get_ts_string()
            print(f"{time_str}: open FDs = {open_fds}")

        previous_rlimit_nofile = rlimit_nofile
        time.sleep(poll_period_sec)



def get_pid_objects(proc_name):

    proc_obj = []
    for proc in psutil.process_iter():

        try:
            pinfo = proc.as_dict(attrs=['pid', 'name', 'create_time'])
            if proc_name.lower() in pinfo['name'].lower():
                proc_obj.append(pinfo)
        except (psutil.NoSuchProcess, psutil.AccessDenied , psutil.ZombieProcess):
            pass
    return proc_obj



def wait_for_process(proc_name):

    proc_obj_list = []
    while True:
        proc_obj_list = get_pid_objects(proc_name)
        if len(proc_obj_list) > 0:
            if len(proc_obj_list) > 1:
                print(f"Process name \'{proc_name}\' matched multiple PIDs. Choosing "
                        "first in list")

            return proc_obj_list[0]
        time.sleep(0.1)
        

def pid_open_fds(pid):

    rlimit_nofile = None
    open_fds = None
    try:
        p = psutil.Process(pid=pid)
        open_fds = p.num_fds()  # Windows: num_handles()
        rlimit_nofile = p.rlimit(psutil.RLIMIT_NOFILE)
    except (psutil.NoSuchProcess, psutil.AccessDenied):
        pass

    return open_fds, rlimit_nofile



def get_ts_string():
    return datetime.now().strftime("%H:%M:%S")



if __name__ == '__main__':

    if len(sys.argv) < 3:
        print('not enough args')

    process_name = sys.argv[1]
    poll_period_sec = int(sys.argv[2])

    main(process_name, poll_period_sec)
