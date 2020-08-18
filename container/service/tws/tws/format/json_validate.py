#!/home/isaac/miniconda3/envs/flaskenv/bin/python

from flask import json
from jsonschema.exceptions import ValidationError
import jsonschema

class ValidateJSON:
    
    def __init__(self):
        self.result = False
        
    def validate_json(self, json_string, schema_dict):
        
        # Interpret json.
        json_data = json.loads(json_string)        
        
        # validate using jsonschema
        try:
            jsonschema.validate(instance=json_data, schema=schema_dict)
        except ValidationError as e:
            # print(e)
            return False
        
        return True
    
    def validate_dict(self, dict_data, schema_dict):
        
        # validate using jsonschema
        try:
            jsonschema.validate(instance=dict_data, schema=schema_dict)
        except ValidationError as e:
            # print(e)
            return False
        
        return True
    
if __name__ == '__main__':
    
    # test
    test_schema = {
        "type" : "object",
        "properties" :
            {
                "parse" : 
                    {
                        "type" : "object",
                        "required" : ["file_path", "flags", "n_threads"]
                    }
            },
        "required" : ["parse", "translate"]
    }
    
    test_data = '''{
        "parse" : {
            "file_path" : "/blah/blah/here.txt",
            "flags" : "-h",
            "n_threads" : 4
        },
        "translate" : "now"
    }'''
    
    v = ValidateJSON()
    j = v.validate_raw(test_data, test_schema)
    print(j)