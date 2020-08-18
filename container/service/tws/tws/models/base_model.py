import inspect
# import abc
# from tws import db

class BaseModel():
    
    def __init__(self):
        all_attribs = inspect.getmembers(
            self, lambda a:not(inspect.isroutine(a)))
        self.attribs = [a[0] for a in all_attribs 
            if not (a[0].startswith('__') and a[0].endswith('__')) 
            and not a[0].startswith('_') 
            # Do not include id if present
            and not a[0].find('id') > -1
            # The following are automatic members.
            and not a[0].find('metadata') > -1 
            and not a[0].find('query') > -1 
            and not a[0].find('query_class') > -1] 
        
    # Pass a dict
    def validate_request(self, req_dict):
        # dict_keys = imm_multi_dict.keys()
        #print('req_dict:', req_dict)
        # print('all keys:', dict_keys)
        for attr in self.attribs:
            #print('key is:', attr)
            if not attr in req_dict:
                #print('key not found!')
                return False
            
        return True
    
    # @abc.abstractmethod
    # def fill(self):
        # pass