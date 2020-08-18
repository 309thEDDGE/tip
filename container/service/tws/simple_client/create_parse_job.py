#!/usr/bin/env python

from simple_client_base import *

class CreateParseJob(SimpleClientBase):

    label_type_dict = {'ch10path': str, 'legacy': bool}
    api_string = '/tip/api/v0.1/jobs/parse'

    def __init__(self):
        SimpleClientBase.__init__(self)
        self.is_parse_job = True
        self.send('post')

if __name__ == '__main__':
    cpj = CreateParseJob()
    
