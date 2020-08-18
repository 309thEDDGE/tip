from exec import Exec
import os, sys
import datetime

def count_errors(stdout_string, printer):
    s = stdout_string.split('\n')
    found_stats = False
    err_count = 0
    for l in s:
        if found_stats:
           if l.find('End') > 0:
               break    
           err_count += int(l.split(' ')[-2])
           printer(l.rstrip())
           
        elif l.find('Comparison Results') > -1:
            #print('found stats!')
            found_stats = True

    if not found_stats:
        printer('count_errors(): did not find summary stats header in:')
        printer(s)
        err_count = int(1e6)
    return err_count

if __name__ == '__main__':

    if len(sys.argv) < 3:
        print('Not enough args')
        sys.exit(0)

    comparison_dir = sys.argv[1]
    test_dir = sys.argv[2]
   

    exec_path = r'C:\Users\DoD_Admin\Documents\repos\tip\tip_scripts\Compare.py'

    file_name = 'validation_translated_' + str(datetime.datetime.now().strftime("%Y%m%d_%H%M%S")) + '.txt'
    log_path = os.path.join(r'D:\test\validation\translated', file_name)
    write_to_log = True

    ff = ''
    lprint = ''
    if write_to_log:
        ff = open(log_path, 'w')
        def lprint(pline):
            ff.write(pline + '\n')
    else:
        def lprint(pline):
            print(pline)

    lprint('comparison dir:\n' + comparison_dir)
    lprint('test dir:\n' + test_dir)

    comp_dir_list = os.listdir(comparison_dir)
    test_dir_list = os.listdir(test_dir)

    #for d in comp_dir_list:
    #    if d not in test_dir_list:
    #        print('\ndir {:s} not in test dir list!'.format(d))
    #        sys.exit(0)

    command = ''
    e = Exec()
    ret_code = 1
    err_count = 0
    
    for d in comp_dir_list:

        if d not in test_dir_list:
            lprint('\ndir {:s} not in test dir list!'.format(d))
            continue

        ret_code = 1
        err_count = 100
        ind = test_dir_list.index(d)
        lprint('\ncomparison: ' + d)
        lprint('test: ' + test_dir_list[ind])
        
        #if d < r'APKWS_recording_10001_080D2020_18131400_18351700_1553_translated_L09.parquet':
        #    continue

        full_comp_dir = os.path.join(comparison_dir, d)
        full_test_dir = os.path.join(test_dir, test_dir_list[ind])

        if len(os.listdir(full_comp_dir)) == 0:
            lprint('Empty comparison directory')
            continue

        if len(os.listdir(full_test_dir)) == 0:
            lprint('Empty test directory')
            continue

        command = ' '.join(['python', exec_path, full_comp_dir, full_test_dir, '--sort', '--no-col-count'])
        #print(command)

        ret_code = e.exec(command)
        if(ret_code != 0):
            lprint('ret_code = {:d}'.format(ret_code))
            break
        stderr, stdout = e.get_output()

        #print('\nstdout:', stdout)
        #print('\nstderr:', stderr)

        err_count = count_errors(stderr, lprint)
        if err_count != 0:
            lprint('!!!!! Error count = {:d} !!!!!'.format(err_count))

    lprint('\n\n')
    for d in test_dir_list:
        if d not in comp_dir_list:
            lprint('test dir {:s} not in comparison dir list!'.format(d))