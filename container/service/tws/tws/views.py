#!/home/isaac/miniconda3/envs/flaskenv/bin/python

# from format.parse_form import ParseForm
# from format.translate_form import TranslateForm
# from format.meta_job import MetaJob
from tws import app, db
from tws.format import *
from tws.models import *
from flask import Blueprint, json, jsonify
from flask import make_response, abort, request

# app = Flask('flask_test')
tip_control = Blueprint('TIP Author', __name__)
pform = ParseForm()
tform = TranslateForm()

jobs = []

@tip_control.route('/')
def index():
    return "hello world!"

@tip_control.errorhandler(404)
def not_found(error):
    return make_response(jsonify({'error': 'Not Found'}), 404)

@tip_control.errorhandler(400)
def bad_request(error):
    return make_response(jsonify({'error': 'Bad Request'}), 400)

@tip_control.errorhandler(405)
def not_allowed(error):
    return make_response(jsonify({'error': 'Not Allowed'}), 405)

@tip_control.errorhandler(500)
def internal_server_error(error):
    return make_response(jsonify({'error': 'Internal Server Error'}), 500)

# @tip_control.errorhandler(204)
# def no_content(error):
#     return make_response(jsonify({'error': 'No Content'}), 204)

@tip_control.route('/tip/api/v0.1/jobs/parse', methods=['POST'])
def create_parse_job():
    
    if not request.json:
        abort(400)
    
    pj = ParseJob()
    if pj.fill(request.json):
        db.session.add(pj)
        db.session.commit()
        jd = JobData(parse_id=pj.id, is_parse_job=True)
        db.session.add(jd)
        db.session.commit()
        return jsonify(jd.as_dict()), 201
    else:
        abort(400)
            
@tip_control.route('/tip/api/v0.1/jobs/translate', methods=['POST'])
def create_translate_job():
    
    if not request.json:
        abort(400)
    
    tj = TranslateJob()
    if tj.fill(request.json):
        db.session.add(tj)
        db.session.commit()
        jd = JobData(translate_id=tj.id, is_parse_job=False)
        db.session.add(jd)
        db.session.commit()
        return jsonify(jd.as_dict()), 201
    else:
        abort(400)
   

# This function returns the status and all other information about
# the top (i.e., most recent) job on the queue. 
@tip_control.route('/tip/api/v0.1/jobs/last', methods=['GET'])
def get_last_job():
        
    jdata = JobData.query.all()
    if len(jdata) > 0:
        last_jdata = jdata[-1]
        if last_jdata.is_parse_job:
            pj = ParseJob.query.get(last_jdata.parse_id)
            return jsonify(last_jdata.as_dict_complete(pj.as_dict())), 200 # ok
        else:
            tj = TranslateJob.query.get(last_jdata.translate_id)
            return jsonify(last_jdata.as_dict_complete(tj.as_dict())), 200 # ok
    else:
        abort(404)

# Returns all submitted jobs.
# TODO: limit based on user? limit based on return count?
@tip_control.route('/tip/api/v0.1/jobs/all', methods=['GET'])
def get_all_jobs():
        
    jdata = JobData.query.all()
    if len(jdata) > 0:
        all_job_dicts = []
        for job in jdata:
            if job.is_parse_job:
                pj = ParseJob.query.get(job.parse_id)
                all_job_dicts.append(job.as_dict_complete(pj.as_dict()))
            else:
                tj = TranslateJob.query.get(job.translate_id)
                all_job_dicts.append(job.as_dict_complete(tj.as_dict()))
        return jsonify(all_job_dicts), 200 # ok
    else:
        abort(404)

# Returns all submitted parse jobs.
@tip_control.route('/tip/api/v0.1/jobs/parse/all', methods=['GET'])
def get_all_parse_jobs():
        
    jdata = JobData.query.all()
    if len(jdata) > 0:
        all_job_dicts = []
        for job in jdata:
            if job.is_parse_job:
                pj = ParseJob.query.get(job.parse_id)
                all_job_dicts.append(job.as_dict_complete(pj.as_dict()))
        return jsonify(all_job_dicts), 200 # ok
    else:
        abort(404)

# Returns all submitted translate jobs.
@tip_control.route('/tip/api/v0.1/jobs/translate/all', methods=['GET'])
def get_all_translate_jobs():
        
    jdata = JobData.query.all()
    if len(jdata) > 0:
        all_job_dicts = []
        for job in jdata:
            if not job.is_parse_job:
                tj = TranslateJob.query.get(job.translate_id)
                all_job_dicts.append(job.as_dict_complete(tj.as_dict()))
        return jsonify(all_job_dicts), 200 # ok
    else:
        abort(404)

# Note: To modify record/row:
# job_data.status = JobStatus.QUEUED
# db.session.commit()

# if __name__ == '__main__':
    # app.run(debug=True)
