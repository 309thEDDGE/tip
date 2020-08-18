#!/home/isaac/miniconda3/envs/flaskenv/bin/python

from tws.format.json_validate import *

class TranslateForm(ValidateJSON):
    
    def __init__(self):
        ValidateJSON.__init__(self)
        # Define the parse job schema.
        self.schema = {
            "type" : "object",
            "properties" : 
                {
                    "pqpath" : {"type" : "string"},
                    "icdpath" : {"type" : "string"},
                    "nthread" : {"type" : "number"},
                    "legacy" : {"type" : "boolean"}
                },
            "required" : [ "pqpath", "icdpath", "nthread", "legacy" ]
        }
        
        # self.default_form_raw = '''{
        #     "pqpath" : "",
        #     "icdpath" : "",
        #     "nthread" : 4
        # }'''
        
        self.default_dict = {'pqpath': '', 'icdpath': '', 'nthread': 4, 'legacy': False}
        
        # self.default_form = json.loads(self.default_form_raw)
        # self.default_form = jsonify(self.default_dict)
        self.form = self.default_dict.copy()
        
    def FormIsCorrect(self, dict_data):
        result = self.validate_dict(dict_data, self.schema)
        if result:
            self.form = dict_data.copy()   
            return True
        else:
            #self.form = None
            return False
        
    def DefaultIsCorrect(self):
        return self.FormIsCorrect(self.default_dict)
    
    def GetJSON(self):
        return json.dumps(self.form)
        

if __name__ == '__main__':
    p = TranslateForm()
    print(p.DefaultIsCorrect())