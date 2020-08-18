#!/usr/bin/env python

from simple_client_base import *

class CreateTranslateJob(SimpleClientBase):

    label_type_dict = {'pqpath': str, 'icdpath': str, 'nthread': int, 'legacy': bool}
    api_string = '/tip/api/v0.1/jobs/translate'

    def __init__(self):
        SimpleClientBase.__init__(self)
        self.is_parse_job = False # It's either a parse job or translate job. Not parse job is translate job.
        self.send('post')

if __name__ == '__main__':
    cpj = CreateTranslateJob()
    
