#!/home/isaac/miniconda3/envs/flaskenv/bin/python


from tws.format import *
from flask import json
from tws.models import *
from tws import app, db

import os
import unittest

class BasicTests(unittest.TestCase):
     
    ############################
    #### setup and teardown ####
    ############################
 
    # executed prior to each test
    def setUp(self):
        app.testing = True
        app.config['TESTING'] = True
        app.config['WTF_CSRF_ENABLED'] = False
        app.config['DEBUG'] = False
        basedir = os.path.basename(os.path.dirname(__file__))
        app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + \
            os.path.join(basedir, 'tws_test.db')
        self.client = app.test_client()
        db.drop_all()
        db.create_all()
 
        # Disable sending emails during unit testing
        #mail.init_app(app)
        self.assertEqual(app.debug, False)
 
    # executed after each test
    def tearDown(self):
        pass
    
    
########################
#### helper methods ####
########################

    def submit_parse_job(self, ch10path, legacy_bool):
        pf = ParseForm()
        pf.form['ch10path'] = ch10path
        pf.form['legacy'] = legacy_bool
        json_form = pf.GetJSON()
        return self.client.post('/tip/api/v0.1/jobs/parse', 
                                data=json_form,
                                content_type='application/json',
                                follow_redirects=True)
        
    def submit_parse_job_missing_field(self, ch10path, legacy_bool, field_to_rm):
        pf = ParseForm()
        pf.form['ch10path'] = ch10path
        pf.form['legacy'] = legacy_bool
        pf.form.pop(field_to_rm)
        json_form = pf.GetJSON()
        # print('json_form:', json_form)
        return self.client.post('/tip/api/v0.1/jobs/parse', 
                                data=json_form,
                                content_type='application/json',
                                follow_redirects=True)
        
    def submit_translate_job(self, pqpath, icdpath, nthread):
        tf = TranslateForm()
        tf.form['pqpath'] = pqpath
        tf.form['icdpath'] = icdpath
        tf.form['nthread'] = nthread
        json_form = tf.GetJSON()
        return self.client.post('/tip/api/v0.1/jobs/translate', 
                                data=json_form,
                                content_type='application/json',
                                follow_redirects=True)
        
    def submit_translate_job_missing_field(self, pqpath, icdpath, nthread, field_to_rm):
        tf = TranslateForm()
        tf.form['pqpath'] = pqpath
        tf.form['icdpath'] = icdpath
        tf.form['nthread'] = nthread
        tf.form.pop(field_to_rm)
        json_form = tf.GetJSON()
        return self.client.post('/tip/api/v0.1/jobs/translate', 
                                data=json_form,
                                content_type='application/json',
                                follow_redirects=True)

    def get_last_job(self):
        return self.client.get('/tip/api/v0.1/jobs/last')

    #def get_last_parse_job(self):
    #    return self.client.get('/tip/api/v0.1/jobs/parse/last')

    #def get_last_translate_job(self):
    #    return self.client.get('/tip/api/v0.1/jobs/translate/last')

    def get_all_jobs(self):
        return self.client.get('/tip/api/v0.1/jobs/all')

    def get_all_parse_jobs(self):
        return self.client.get('/tip/api/v0.1/jobs/parse/all')

    def get_all_translate_jobs(self):
        return self.client.get('/tip/api/v0.1/jobs/translate/all')
 
###############
#### tests ####
###############
 
    def test_main_page(self):
        response = self.client.get('/', follow_redirects=True)
        self.assertEqual(response.status_code, 200)
        
    def test_submit_good_parse_job(self):
        ch10path = '/home/user/path/blah.ch10'
        response = self.submit_parse_job(ch10path, False)        

        # Check for proper return code.
        self.assertEqual(response.status_code, 201)

        # Check response data. Not sure if I need these data returned.
        response_dict = json.loads(response.data)
        self.assertTrue(response_dict['is_parse_job'])

        # Check last entry in database.
        jd = JobData.query.all()[-1]
        self.assertTrue(jd.is_parse_job)

        # Check that database contains contains correct data.
        # Only checking the ch10 path now.
        pj = ParseJob.query.get(jd.parse_id)
        self.assertEqual(ch10path, pj.ch10path)
        
    def test_submit_bad_parse_job_missing_field(self):
        response = self.submit_parse_job_missing_field('/home/user/path/blah.ch10', False, 'legacy')        
        response_dict = json.loads(response.data)
        self.assertEqual(response.status_code, 400)
         
    def test_submit_good_translate_job(self):
        pqpath = '/home/user/path/blah.parquet'
        icdpath = '/home/user/icd/icd.txt'
        nthread = 5
        response = self.submit_translate_job(pqpath, icdpath, nthread)        

        # Check return code        
        self.assertEqual(response.status_code, 201)

        # Check response -- not sure if response is necesssary.
        response_dict = json.loads(response.data)
        self.assertFalse(response_dict['is_parse_job'])

        # Check last entry in database.
        jd = JobData.query.all()[-1]
        self.assertFalse(jd.is_parse_job)

        # Check that database contains contains correct data.
        tj = TranslateJob.query.get(jd.translate_id)
        self.assertEqual(pqpath, tj.pqpath)
        self.assertEqual(icdpath, tj.icdpath)
        self.assertEqual(nthread, tj.nthread)
        
    def test_submit_bad_translate_job_missing_field(self):
        response = self.submit_translate_job_missing_field('/home/user/path/blah.parquet', '/home/user/icd/icd.txt', 6, 'icdpath')        
        # print('response.data: ', response.data)
        response_dict = json.loads(response.data)
        self.assertEqual(response.status_code, 400)

    def test_get_last_job_empty(self):
        response = self.get_last_job()
        
        # Get 404 not found if queue is empty.
        self.assertEqual(response.status_code, 404)
        
    def test_get_last_job_one_parse_job(self):
        # Create a parse job and submit it.
        fakech10path = '/ch10/test/path/file.ch10'
        response = self.submit_parse_job(fakech10path, False)
        
        # Submission must be good ==> code = 201
        self.assertEqual(response.status_code, 201)
        
        # GET the top job on the queue, currently the job we just submitted.
        response = self.get_last_job()
        
        # Confirm good return code.
        self.assertEqual(response.status_code, 200)
        
        # Confirm it's a parse job, path is same is submitted above, and legacy is False.
        response_dict = json.loads(response.data)
        self.assertTrue(response_dict['meta']['is_parse_job'])
        self.assertEqual(response_dict['job']['ch10path'], fakech10path)
        self.assertFalse(response_dict['job']['legacy'])
        
    def test_get_last_job_mult_jobs(self):
        # Create a parse job and submit it.
        fakech10path1 = '/ch10/test/path/file.ch10'
        response = self.submit_parse_job(fakech10path1, False)
        self.assertEqual(response.status_code, 201)

        # Create another parse job.
        fakech10path2 = '/ch10/test2/long_path/file2.ch10'
        response = self.submit_parse_job(fakech10path2, True)
        self.assertEqual(response.status_code, 201)
        
        # Create a translate job and submit it.
        nthread = 10
        pqpath = '/home/user/su/data_out/foobar.parquet'
        icdpath = '/my/icd/path/icd.txt'
        response = self.submit_translate_job(pqpath, icdpath, nthread)
        self.assertEqual(response.status_code, 201)
        
        # GET the top job on the queue, which ought to be the translate job.
        # Validate a couple of fields.
        response = self.get_last_job()
        self.assertEqual(response.status_code, 200)

        data = json.loads(response.data)
        #print(data)
        self.assertFalse(data['meta']['is_parse_job'])
        self.assertEqual(data['job']['pqpath'], pqpath)
        self.assertEqual(data['job']['nthread'], nthread)

    def test_get_all_jobs(self):
         # Create a parse job and submit it.
        fakech10path1 = '/ch10/test/path/file.ch10'
        response = self.submit_parse_job(fakech10path1, False)
        self.assertEqual(response.status_code, 201)

        # Create another parse job.
        fakech10path2 = '/ch10/test2/long_path/file2.ch10'
        response = self.submit_parse_job(fakech10path2, True)
        self.assertEqual(response.status_code, 201)

        response = self.get_all_jobs()
        self.assertEqual(response.status_code, 200)

        data = json.loads(response.data)
        self.assertEqual(len(data), 2)

        self.assertEqual(fakech10path1, data[0]['job']['ch10path'])
        self.assertEqual(fakech10path2, data[1]['job']['ch10path'])

def test_get_all_jobs_of_type(self):
         # Create a parse job and submit it.
        fakech10path1 = '/ch10/test/path/file.ch10'
        response = self.submit_parse_job(fakech10path1, False)
        self.assertEqual(response.status_code, 201)

        # Create a translate job and submit it.
        nthread = 3
        pqpath = '/home/user/su/data_out/foobar.parquet'
        icdpath = '/my/icd/path/icd.txt'
        response = self.submit_translate_job(pqpath, icdpath, nthread)
        self.assertEqual(response.status_code, 201)

        # Create another parse job.
        fakech10path2 = '/ch10/test2/long_path/file2.ch10'
        response = self.submit_parse_job(fakech10path2, True)
        self.assertEqual(response.status_code, 201)

        response = self.get_all_parse_jobs()
        self.assertEqual(response.status_code, 200)

        data = json.loads(response.data)
        self.assertEqual(len(data), 2)

        self.assertEqual(fakech10path1, data[0]['job']['ch10path'])
        self.assertEqual(fakech10path2, data[1]['job']['ch10path'])

        response = self.get_all_translate_jobs()
        self.assertEqual(response.status_code, 200)

        data = json.loads(response.data)
        self.assertEqual(len(data), 1)

        self.assertEqual(nthread, data[0]['job']['nthread'])
        self.assertEqual(icdpath, data[0]['job']['icdpath'])

 
if __name__ == "__main__":
    unittest.main()