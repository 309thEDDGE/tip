import os
from pathlib import Path
from tip_scripts.e2e_validation.directory_validation import DirectoryValidation
from tip_scripts.e2e_validation.pqpq_translated1553_validation import PqPqTranslated1553Validation
from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation

class PqPqTranslated1553DirValidation(DirectoryValidation):

    def __init__(self, truth_dir, test_dir, pqcompare_exec_path, bincompare_exec_path):
        DirectoryValidation.__init__(self, 'PqPqTranslated1553DirValidation')
        self.ready_to_validate = self.set_directory_paths(truth_dir, test_dir)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.bincompare_exec_path = bincompare_exec_path

        #if self.ready_to_validate:
        self._create_validation_objects()

    #def _validate_top_level_dirs(self):

    #    # Do both truth and test dirs exist?
    #    if not os.path.isdir(self.truth_dir_path):
    #        print('Top level truth dir does not exist: {:s}'.format(self.truth_dir_path))
    #        self.top_level_dirs_validated = False
    #        return
       
    #    if not os.path.isdir(self.test_dir_path):
    #        print('Top level test dir does not exist: {:s}'.format(self.truth_dir_path))
    #        self.top_level_dirs_validated = False
    #        return

    #    # Do both truth and test dirs contain files?
    #    #self.truth_dir_list = [os.path.join(self.truth_dir, x) for x in os.listdir(self.truth_dir_path)]
    #    self.truth_dir_list = os.listdir(self.truth_dir_path)
    #    if len(self.truth_dir_list) == 0:
    #        print('Top level truth dir is empty: {:s}'.format(self.truth_dir_path))
    #        self.top_level_dirs_validated = False
    #        return

    #    #self.test_dir_list = [os.path.join(self.test_dir, x) for x in os.listdir(self.test_dir_path)]
    #    self.test_dir_list = os.listdir(self.test_dir_path)
    #    if len(self.test_dir_list) == 0:
    #        print('Top level test dir is empty: {:s}'.format(self.test_dir_path))
    #        self.top_level_dirs_validated = False
    #        return

    #    self.top_level_dirs_validated = True

    def _create_validation_objects(self):

        # Custom override
        extension_list = ['.txt', '.yml', '.yaml']
        metadata_files = [] 

        # Loop over msg-specific directories in truth list and create
        # translated 1553 msg data validation objects.
        truth_dir = Path(self.truth_path)
        test_dir = Path(self.test_path)
        for truthfile in os.listdir(self.truth_path):

            # Find metadata files, only if yaml or txt file.
            truth_file_path = Path(truthfile)
            if truth_file_path.suffix in extension_list:
                metadata_files.append(truth_file_path)
                continue

            if (truth_dir / truth_file_path).is_dir():
                self.validation_objects.append(PqPqTranslated1553Validation(
                    str(truth_dir / truth_file_path),
                    str(test_dir / truth_file_path),
                    self.pqcompare_exec_path))

        for truth_md_path in metadata_files:

            if truth_md_path.suffix in ['.yml', '.yaml']:
                self.validation_objects.append(YmlYmlValidation(
                    str(truth_dir / truth_md_path),
                    str(test_dir / truth_md_path)))
            elif truth_md_path.suffix in ['.txt']:
                self.validation_objects.append(TxtTxtValidation(
                    str(truth_dir / truth_md_path),
                    str(test_dir / truth_md_path),
                    self.bincompare_exec_path))


