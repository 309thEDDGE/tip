#
# Install deepdiff:
# pip install 'deepdiff[murmur]'
#
from deepdiff import DeepDiff

'''
DeepDiffCompare

Wrapper class around DeepDiff, returns:

'NULL' - objects can't be compared, or comparison is
not completed for some reason

'PASS' - objects are compared and found to be equal

'FAIL' - objects are compared and found to be unequal
'''

class DeepDiffCompare:
    
   def __init__(self):
       pass
   
   def Compare(self, truth_obj, test_obj):
       
       
       ddiff = DeepDiff(truth_obj, test_obj, ignore_order=False,
                        report_repetition=True)
       
       if len(ddiff) == 0:
           return True
       else:
           print(ddiff)
           return False