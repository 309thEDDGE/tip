#!/usr/bin/env python

from simple_client_base import *

class GetAllTranslateJobs(SimpleClientBase):

    label_type_dict = {}
    api_string = '/tip/api/v0.1/jobs/translate/all'

    def __init__(self):
        SimpleClientBase.__init__(self)
        self.is_parse_job = True
        self.send('get')

if __name__ == '__main__':
    cpj = GetAllTranslateJobs()
    
