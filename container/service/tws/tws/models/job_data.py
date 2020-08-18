from tws import db
#from tws.models.base_model import BaseModel
from tws.models.parse_job import ParseJob
from tws.models.translate_job import TranslateJob
from datetime import datetime

class JobStatus:
    
    def __init__(self):
        self.QUEUED = 'QUEUED'
        self.IN_PROGRESS = 'IN_PROGRESS'
        self.FAILED = 'FAILED'
        self.COMPLETE = 'COMPLETE'
        self.NONE = 'NONE'
        self.CREATED = 'CREATED'

class JobData(db.Model):

    jstat = JobStatus()

    id = db.Column(db.Integer, primary_key=True)
    status = db.Column(db.String, index=True, unique=False, default=jstat.CREATED)
    is_parse_job = db.Column(db.Boolean, index=False, unique=False, default=False)
    timestamp = db.Column(db.DateTime, index=True, default=datetime.utcnow)
    parse_id = db.Column(db.Integer, db.ForeignKey('parse_job.id'), default=None)
    translate_id = db.Column(db.Integer, db.ForeignKey('translate_job.id'), default=None)
    # column for user, if necessary. Use foreignkey.

    def as_dict(self):
        return {'status': self.status, 'is_parse_job': self.is_parse_job, \
                'timestamp': self.timestamp, 'parse_id': self.parse_id, 'translate_id': self.translate_id}

    def as_dict_complete(self, job_dict):
        return {'meta': self.as_dict(), 'job': job_dict}

    def __repr__(self):
        return str(self.as_dict())