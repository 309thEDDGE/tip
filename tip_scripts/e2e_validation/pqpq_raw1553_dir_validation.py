import os
from pathlib import Path
from tip_scripts.e2e_validation.pqpq_raw1553_validation import PqPqRaw1553Validation
from tip_scripts.e2e_validation.ymlyml_validation import YmlYmlValidation
from tip_scripts.e2e_validation.txttxt_validation import TxtTxtValidation

class PqPqRaw1553DirValidation():

    def __init__(self, truth_path, test_path, pqcompare_exec_path, bincompare_exec_path):
        self.truth_path = truth_path
        self.test_path = test_path
        self.pqcompare_exec_path = pqcompare_exec_path
        self.bincompare_exec_path = bincompare_exec_path
        self.md_validation_objects = []
        self.all_passed = None
        self.pq_validation_object = PqPqRaw1553Validation(self.truth_path, self.test_path, self.pqcompare_exec_path)
        self._create_metadata_objects()

    def validate(self, print_obj):

        none_count = 0
        true_count = 0
        false_count = 0

        # Validate pq data.
        info = '\n' + str(self.pq_validation_object)
        print_obj(info)
        print(info)
        result = self.pq_validation_object.validate()
        print_obj('Validated: {}'.format(result))

        if result is None:
            none_count += 1
        elif result:
            true_count += 1
        else:
            false_count += 1

        # Validate metadata objects
        for md_validation_obj in self.md_validation_objects:

            info = '\n' + str(md_validation_obj)
            print_obj(info)
            print(info)
            result = md_validation_obj.validate()
            print_obj('Validated: {}'.format(result))    

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


    def _create_metadata_objects(self):

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
                    self.md_validation_objects.append(YmlYmlValidation(
                        str(truth_dir / truth_md_path),
                        str(test_dir / truth_md_path),
                        'YmlYmlValidation'))
                elif truth_md_path.suffix in ['.txt']:
                    self.md_validation_objects.append(TxtTxtValidation(
                        str(truth_dir / truth_md_path),
                        str(test_dir / truth_md_path),
                        'TxtTxtValidation',
                        self.bincompare_exec_path))