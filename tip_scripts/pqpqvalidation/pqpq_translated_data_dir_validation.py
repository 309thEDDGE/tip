import os
from tip_scripts.pqpqvalidation.pqpq_translated_data_validation import PqPqTranslatedDataValidation

class PqPqTranslatedDataDirValidation:

    def __init__(self, truth_dir, test_dir, expected_translation_dir_name, exec_path):
        self.truth_dir = truth_dir
        self.test_dir = test_dir
        self.expected_translation_dir_name = expected_translation_dir_name
        self.exec_path = exec_path
        self.truth_dir_path = os.path.join(self.truth_dir, self.expected_translation_dir_name)
        self.test_dir_path = os.path.join(self.test_dir, self.expected_translation_dir_name)
        self.top_level_dirs_validated = False
        self.truth_dir_list = []
        self.test_dir_list = []
        self.validation_objects = []
        self.all_passed = None

        self._validate_top_level_dirs()
        if self.top_level_dirs_validated:
            self._create_validation_objects()

    def _validate_top_level_dirs(self):

        # Do both truth and test dirs exist?
        if not os.path.isdir(self.truth_dir_path):
            print('Top level truth dir does not exist: {:s}'.format(self.truth_dir_path))
            self.top_level_dirs_validated = False
            return
       
        if not os.path.isdir(self.test_dir_path):
            print('Top level test dir does not exist: {:s}'.format(self.truth_dir_path))
            self.top_level_dirs_validated = False
            return

        # Do both truth and test dirs contain files?
        #self.truth_dir_list = [os.path.join(self.truth_dir, x) for x in os.listdir(self.truth_dir_path)]
        self.truth_dir_list = os.listdir(self.truth_dir_path)
        if len(self.truth_dir_list) == 0:
            print('Top level truth dir is empty: {:s}'.format(self.truth_dir_path))
            self.top_level_dirs_validated = False
            return

        #self.test_dir_list = [os.path.join(self.test_dir, x) for x in os.listdir(self.test_dir_path)]
        self.test_dir_list = os.listdir(self.test_dir_path)
        if len(self.test_dir_list) == 0:
            print('Top level test dir is empty: {:s}'.format(self.test_dir_path))
            self.top_level_dirs_validated = False
            return

        self.top_level_dirs_validated = True

    def _create_validation_objects(self):

        # Loop over msg-specific directories in truth list and create
        # validation objects.
        for truthfile in self.truth_dir_list:

            # Ignore if yaml or txt file.
            if (truthfile.find('.yaml') > 0 or truthfile.find('.yml') > 0 or
                truthfile.find('.txt') > 0):
                print('Skipping {:s}'.format(truthfile))
                continue

            self.validation_objects.append(PqPqTranslatedDataValidation(
                os.path.join(self.truth_dir_path, truthfile),
                os.path.join(self.test_dir_path, truthfile),
                self.exec_path))

    def validate(self, print_obj, save_stdout):

        none_count = 0
        true_count = 0
        false_count = 0

        if self.top_level_dirs_validated:
            
            for vobj in self.validation_objects:

                info = '\n' + str(vobj)
                print_obj(info)
                print(info)
                result = vobj.validate()
                print_obj('Validated: {}'.format(result))

                if save_stdout:
                    stdout, stderr = vobj.get_validation_output()
                    print_obj('\nstdout:')
                    print_obj(stdout)
                    print_obj('\nstderr:')
                    print_obj(stderr)

                if result is None:
                    none_count += 1
                elif result:
                    true_count += 1
                else:
                    false_count += 1

            if none_count > 0:
                self.all_passed = None
            elif false_count == 0:
                self.all_passed = True
            else:
                self.all_passed = False
