import sys, os
import subprocess

class Exec:
    def __init__(self, *args, **kwargs):
        self.stdout = ''
        self.stderr = ''
        self.command_str = ''
        self.command_list = []

    def exec(self, command_string):
        self.command_str = command_string
        self.command_list = self.command_str.split(' ')
        #print('Exec::exec(): command list = ', self.command_list)
        proc = subprocess.Popen(self.command_list, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate()
        self.stdout = stdout.decode('utf-8')
        self.stderr = stderr.decode('utf-8')

        return proc.returncode

    def exec_list(self, command_list, cwd=None, print_stdout=True):
        sys.stdout.flush()
        self.command_str = ''
        self.command_list = command_list
        if print_stdout:
            print('\nExecuting:\n\'{:s}\'\n'.format(' '.join(self.command_list)))
        proc = subprocess.Popen(self.command_list, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, cwd=cwd)
        symbol_count = 20
        stdout = ''
        stderr = ''
        if print_stdout:
            print('{:s} {:s} {:s}'.format(symbol_count * '=', 
                                          os.path.basename(self.command_list[0]) + ' OUTPUT', 
                                          symbol_count * '='))

        rawstdout = ''
        rawstderr = ''
        decodestdout = ''
        while True:
            # Note: do not read from stderr because it blocks
            # if no data are present.
            decodestdout = proc.stdout.readline().decode()
            if decodestdout == '' and proc.poll() is not None:
                break
            if decodestdout:
                stdout += decodestdout
                if print_stdout:
                    print(decodestdout.strip())
                sys.stdout.flush()

        rawstdout, rawstderr = proc.communicate()
        stderr = rawstderr.decode()
        if print_stdout:
            print('{:s} {:s} {:s}\n'.format(symbol_count * '=', 'END OUTPUT', symbol_count * '='))

        self.stdout = stdout
        self.stderr = stderr

        return proc.poll()

    def get_output(self):
        return self.stdout, self.stderr