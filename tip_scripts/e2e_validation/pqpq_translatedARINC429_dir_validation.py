import os
from pathlib import Path
from tip_scripts.e2e_validation import config
from tip_scripts.e2e_validation.directory_validation import DirectoryValidation
from tip_scripts.e2e_validation.pqpq_translatedARINC429_validation import PqPqTranslatedARINC429Validation
if config.COMPARE_YAML:
    from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
    from tip_scripts.e2e_validation.yaml_compare import exclude_from_comparison_funcs as exclude_funcs
from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation

class PqPqTranslatedARINC429DirValidation(DirectoryValidation):

    def __init__(self, truth_dir, test_dir, pqcompare_exec_path, bincompare_exec_path):
        DirectoryValidation.__init__(self, 'PqPqTranslatedARINC429DirValidation')
        self.ready_to_validate = self.set_directory_paths(truth_dir, test_dir)
        self.pqcompare_exec_path = pqcompare_exec_path
        self.bincompare_exec_path = bincompare_exec_path

        self._create_validation_objects()

    def _create_validation_objects(self):

        # Custom override

        if self.truth_dir_exists == True:

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
                    self.validation_objects.append(PqPqTranslatedARINC429Validation(
                        str(truth_dir / truth_file_path),
                        str(test_dir / truth_file_path),
                        self.pqcompare_exec_path))

            for truth_md_path in metadata_files:

                if truth_md_path.suffix in ['.yml', '.yaml']:
                    if config.COMPARE_YAML:
                        self.validation_objects.append(YmlYmlValidation(
                            str(truth_dir / truth_md_path),
                            str(test_dir / truth_md_path),
                            exclude_func=exclude_funcs.translated_arinc429f0
                            ))
                elif truth_md_path.suffix in ['.txt']:
                    self.validation_objects.append(TxtTxtValidation(
                        str(truth_dir / truth_md_path),
                        str(test_dir / truth_md_path),
                        self.bincompare_exec_path))


