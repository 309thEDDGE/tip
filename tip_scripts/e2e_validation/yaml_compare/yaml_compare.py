import os, sys
import yaml
import json
from tip_scripts.e2e_validation.yaml_compare.deepdiff_compare import DeepDiffCompare

'''
YamlCompare

Input 
'''

class YamlCompare:
    
    def __init__(self, truth_fp, test_fp):
        self.truth_fp = truth_fp
        self.test_fp = test_fp
        
    def Compare(self, verbosity=0, exclude_func=None):
        '''
        Compare
        
        Read both truth and test yaml documents, 
        ingest into dictionaries and compare the content
        mapped to each top-level key.
        
        Print erroneous comparison only, unless 
        verbosity is set greater than zero. 
        
        Return: dictionary containing results, map of 
        top-level keys to string result and total result
        mapped to key 'total'.
        '''
        if not self._file_check():
            return None
        
        with open(self.truth_fp, 'r') as f:
            truth_dict = yaml.load(f, Loader=yaml.FullLoader)
            
        with open(self.test_fp, 'r') as f:
            test_dict = yaml.load(f, Loader=yaml.FullLoader)
            
        if verbosity > 0:
            print('truth_dict:', truth_dict)
            print('test_dict:', test_dict)
            print('')
            
        return self._do_comparison(truth_dict, test_dict, verbosity, exclude_func=exclude_func)
    
    def _set_pass_fail_strings(self, results_dict):
        
        if len(results_dict) == 0:
            results_dict['total'] = self._ret_string(None)
            return results_dict
        
        result_vals = list(results_dict.values())
        
        total_result = None
        if result_vals.count(None) > 0:
            total_result = None
        elif result_vals.count(False) > 0:
            total_result = False
        else:
            total_result = True
            
        results_dict['total'] = total_result
            
        return results_dict


    def _do_comparison(self, truth_dict, test_dict, v, exclude_func=None):
        
        truth_keys = sorted(list(truth_dict.keys()))
        test_keys = sorted(list(test_dict.keys()))
        
        single_result = None
        results = {}
        ddc = DeepDiffCompare()
        return ddc.Compare(truth_dict, test_dict, exclude_func=exclude_func)
        
    def _file_check(self):
        
        if not os.path.isfile(self.truth_fp):
            return False
        
        if not os.path.isfile(self.test_fp):
            return False
        
        return True
        
    
    def _ret_string(self, val):
        
        if val is None:
            return 'NULL'
        elif val == True:
            return 'PASS'
        elif val == False:
            return 'FAIL'
        else:
            return 'BAD RESULT'
        
if __name__ == '__main__':
    
    yc = YamlCompare(sys.argv[1], sys.argv[2])
    res = yc.Compare()
    print('\njson:')
    json.dump(res, sys.stdout)
    print('')