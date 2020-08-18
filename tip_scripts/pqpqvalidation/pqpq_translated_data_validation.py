from tip_scripts.pqpqvalidation import validation_base

class PqPqTranslatedDataValidation(validation_base.ValidationBase):

    def __init__(self, truth_path, test_path, exec_path):
        validation_base.ValidationBase.__init__(self)
        self.ready_to_validate = self.set_paths(truth_path, test_path, True)
        self.exec_path = exec_path

    def validate(self):
        return self.do_validation(self.exec_path)