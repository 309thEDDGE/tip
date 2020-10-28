import os, sys
import numpy as np
from tip_scripts.exec import Exec
from shutil import rmtree

class RunCLProcess(Exec):

    def __init__(self, *args, **kwargs):
        Exec.__init__(self, *args, **kwargs)
        if 'debug' in kwargs.keys():
            self.debug = kwargs['debug']
        else:
            self.debug = 0
        self.exec_path = ''
        self.args = []
        self.ret_val = 0
        self.success = False
        self.file_paths = []
        self.dir_paths = []
        self.stdout_must_contain = []
        #self.stdout_may_contain = []
        self.stderr_must_contain = []
        #self.stderr_may_contain = []
        self.required_output_dirs = []
        self.required_output_files = []

    def set_executable_path(self, exec_path):
        self.file_paths.append(exec_path)
        self.exec_path = exec_path

    def _add_to_list(self, new_vals, thelist):
        if type(new_vals) is list:
            strlist = [str(x) for x in new_vals]
            thelist.extend(strlist)
        else:
            thelist.append(str(new_vals))

    def _check_paths(self):
        for p in self.file_paths:
            if not os.path.isfile(p):
                print('File path not present: \'{:s}\''.format(p))
                return False
        for d in self.dir_paths:
            if not os.path.isdir(d):
                print('Directory path not present: \'{:s}\''.format(d))
                return False
        return True

    def get_exec_time(self):
        return self.exec_time

    def add_dir_path_argument(self, new_arg):
        self._add_to_list(new_arg, self.dir_paths)
        self._add_to_list(new_arg, self.args)

    def add_file_path_argument(self, new_arg):
        self._add_to_list(new_arg, self.file_paths)
        self._add_to_list(new_arg, self.args)

    def add_command_argument(self, new_arg):
        self._add_to_list(new_arg, self.args)

    def set_stdout_must_contain(self, must_contain_str):
        self._add_to_list(must_contain_str, self.stdout_must_contain)

    def set_stderr_must_contain(self, must_contain_str):
        self._add_to_list(must_contain_str, self.stderr_must_contain)

    def register_output_dir(self, output_dir):
        self._add_to_list(output_dir, self.required_output_dirs)

    def register_output_file(self, output_file):
        self._add_to_list(output_file, self.required_output_files)

    def _string_is_present(self, instr, searchstr):
        if instr.find(searchstr) > -1:
            return True
        else:
            return False

    def _update_find_list(self, find_list, update_list):
        for i, b in enumerate(update_list):
            if b:
                find_list[i] = b

    def _all_true(self, bool_list):
        if np.sum(np.array([int(x) for x in bool_list])) == len(bool_list):
            return True
        else:
            return False

    def _check_text_output(self, print_stdout=True):
            self.success = True
            stdout = self.stdout
            stderr = self.stderr

            # Check for required stdout strings.
            find_list = [False] * len(self.stdout_must_contain)
            if self.debug > 1:
                print('stdout_must_contain: ', self.stdout_must_contain)
                print('find_list: ', find_list)
            if len(self.stdout_must_contain) > 0:
                for l in stdout.split('\n'):
                    self._update_find_list(find_list, [self._string_is_present(l, x) for x in self.stdout_must_contain])
                    if self.debug > 0:
                        print('\nstdout line: ', l)
                        print('find_list: ', find_list)

                if not self._all_true(find_list):
                    self.success = False
                    if print_stdout:
                        print('{:s}: failed to find stdout success strings'.format(self.exec_path))
                    return
        
            # Check for required stderr strings
            find_list = [False] * len(self.stderr_must_contain)
            if len(self.stderr_must_contain) > 0:
                for l in stderr.split('\n'):
                    self._update_find_list(find_list, [self._string_is_present(l, x) for x in self.stderr_must_contain])

                if not self._all_true(find_list):
                    self.success = False
                    if print_stdout:
                        print('{:s}: failed to find stderr success strings'.format(self.exec_path))
                    return

    def _check_output(self):
        for afile in self.required_output_files:
            if not os.path.isfile(afile):
                print('{:s} failed: file {:s} does not exist'.format(self.exec_path, afile))
                self.success = False
                return
        for adir in self.required_output_dirs:
            if not os.path.isdir(adir):
                print('{:s} failed: directory {:s} does not exist'.format(self.exec_path, adir))
                self.success = False
                return

    def run(self, dry_run, cwd=None, print_stdout=True):
        '''
        Run CL process.

        Return True if process is called. False if not.
        '''
        if not self._check_paths():
            print('RunCLProcess.run(): _check_paths failed!')
            self.success = False
            return False

        command_list = [self.exec_path]
        self._add_to_list(self.args, command_list)
        
        if not dry_run:
            self.ret_val = self.exec_list(command_list, cwd=cwd, print_stdout=print_stdout)
            if self.ret_val != 0:
                print('{:s} failed: return value {:d}'.format(self.exec_path, int(self.ret_val)))
                return True

            self._check_text_output(print_stdout=print_stdout)

            if self.success:
                print('SUCCESS')
            else: 
                print('FAILURE')
            
            return True

        return False

    def get_return_value(self):
        return self.ret_val

    def get_stdout_lines_matching(self, match_str):

        lines = []
        for l in self.stdout.split('\n'):
            if match_str in l:
                lines.append(l)
        return lines

    def have_output_success(self):
        return self.success

    def output_files_exist(self):
        for f in self.required_output_files:
            if os.path.isfile(f):
                return True
        return False

    def message_output_files_exist(self):
        print('\nAt least one of the following output files exist: ', self.required_output_files)

    def remove_existing_output_files(self):
        for f in self.required_output_files:
            if os.path.isfile(f):
                print('Removing path: {:s}'.format(f))
                os.remove(f)

    def output_dirs_exist(self):
        for d in self.required_output_dirs:
            if os.path.isdir(d):
                return True
        return False

    def message_output_dirs_exist(self):
        print('\nAt least one of the following output directories exist: ', self.required_output_dirs)

    def remove_existing_output_dirs(self):
        for f in self.required_output_dirs:
            if os.path.isdir(f):
                print('Removing path and contents: {:s}'.format(f))
                os.system('rmdir /Q /S {:s}'.format(f))
                #self.remove_path_and_contents(f)

    def remove_path_and_contents(self, p):
        for root, dirs, files in os.walk(p):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                # Remove any files present in the directory first.
                self.remove_path_and_contents(os.path.join(root, name))
                os.rmdir(os.path.join(root, name))
