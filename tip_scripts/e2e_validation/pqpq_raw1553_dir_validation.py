import os
from pathlib import Path
from tip_scripts.e2e_validation import config
from tip_scripts.e2e_validation.directory_validation import DirectoryValidation
from tip_scripts.e2e_validation.pqpq_raw1553_validation import PqPqRaw1553Validation
from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation
if config.COMPARE_YAML:
    from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
    from tip_scripts.e2e_validation.yaml_compare import exclude_from_comparison_funcs as exclude_funcs

class PqPqRaw1553DirValidation(DirectoryValidation):

    def __init__(self, truth_path, test_path, pqcompare_exec_path, bincompare_exec_path):
        DirectoryValidation.__init__(self, 'PqPqRaw1553DirValidation')
        self.ready_to_validate = self.set_directory_paths(truth_path, test_path)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.bincompare_exec_path = bincompare_exec_path

        #if self.ready_to_validate:
        self._create_validation_objects()


    def _create_validation_objects(self):

        # Custom override

        # Add pq comparison object first.
        self.validation_objects.append(
            PqPqRaw1553Validation(self.truth_path, self.test_path, self.pqcompare_exec_path))

        # Add metadata objects.
        if self.truth_dir_exists == True:

            extension_list = ['.txt', '.yml', '.yaml']
            # Search list of files for those with extension matching
            # an entry in the extension_list, which includes the leading '.'
            # (ex: ['.txt', '.yml']).
            truth_metadata_files = [x for x in os.listdir(self.truth_path) if Path(x).suffix in extension_list]

            if len(extension_list) > 0:

                truth_dir = Path(self.truth_path)
                test_dir = Path(self.test_path)
                for truth_md_file in truth_metadata_files:

                    truth_md_path = Path(truth_md_file)
                    if truth_md_path.suffix in ['.yml', '.yaml']:
                        if config.COMPARE_YAML:
                            self.validation_objects.append(YmlYmlValidation(
                                str(truth_dir / truth_md_path),
                                str(test_dir / truth_md_path),
                                exclude_func=exclude_funcs.parsed_1553f1))
                    elif truth_md_path.suffix in ['.txt']:
                        self.validation_objects.append(TxtTxtValidation(
                            str(truth_dir / truth_md_path),
                            str(test_dir / truth_md_path),
                            self.bincompare_exec_path))