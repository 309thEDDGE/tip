#!/home/isaac/miniconda3/envs/flaskenv/bin/python
from flask import json

class JobStatus:
    
    def __init__(self):
        self.QUEUED = 'QUEUED'
        self.IN_PROGRESS = 'IN_PROGRESS'
        self.FAILED = 'FAILED'
        self.COMPLETE = 'COMPLETE'
        self.NONE = 'NONE'
        self.CREATED = 'CREATED'


class MetaJob:
    
    def __init__(self, dict_form):
        
        # Form type params
        self.parse_job_indicator_field = 'ch10path'
        self.is_parse_job = False
        if self.parse_job_indicator_field in dict_form.keys():
            # print('MetaJob: form is parse job')
            self.is_parse_job = True
            
        # Meta params
        self.status = JobStatus()

        self.form = {}
        self.form['status'] = self.status.CREATED
        self.form['is_parse_job'] = self.is_parse_job
        self.form['job'] = dict_form
        

            
    
        