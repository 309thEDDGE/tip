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
           #printer(l.rstrip())
           
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

    file_name = 'validation_raw_' + str(datetime.datetime.now().strftime("%Y%m%d_%H%M%S")) + '.txt'
    log_path = os.path.join(r'D:\test\validation\raw', file_name)
    write_to_log = True

    ff = ''
    lprint = ''
    if write_to_log:
        ff = open(log_path, 'w')
        def lprint(pline):
            ff.write(pline + '\n')
            ff.flush()
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

        # Neglect metadata.
        if d.find('metadata') > -1:
            continue

        if d not in test_dir_list:
            lprint('\ndir {:s} not in test dir list!'.format(d))
            continue

        ret_code = 1
        err_count = 100
        ind = test_dir_list.index(d)
        lprint('\ncomparison: ' + d)
        lprint('test: ' + test_dir_list[ind])

        command = ' '.join(['python', exec_path, os.path.join(comparison_dir, d), 
                            os.path.join(test_dir, test_dir_list[ind]), '--no-sort', 
                            '--no-col-count', '--no-row-count'])
        #print(command)

        ret_code = e.exec(command)
        if(ret_code != 0):
            lprint('ret_code = {:d}'.format(ret_code))
            break
        stderr, stdout = e.get_output()

        lprint('\nstdout:\n')
        lprint(stdout)
        lprint('\nstderr:\n')
        lprint(stderr)

        err_count = count_errors(stderr, lprint)
        if err_count != 0:
            lprint('!!!!! Error count = {:d} !!!!!'.format(err_count))

    lprint('\n\n')
    for d in test_dir_list:
        # Neglect metadata.
        if d.find('metadata') > -1:
            continue

        if d not in comp_dir_list:
            lprint('test dir {:s} not in comparison dir list!'.format(d))