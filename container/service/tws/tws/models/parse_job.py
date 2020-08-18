
# from app import db
from tws import db
from tws.models.base_model import BaseModel

class ParseJob(BaseModel, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    ch10path = db.Column(db.String, index=True, unique=False)
    legacy = db.Column(db.Boolean, index=False, unique=False)
    
    def __init__(self):
        BaseModel.__init__(self)
        #print('attribs:', self.attribs)
    
    def as_dict(self):
        return {'id': self.id, 'ch10path': self.ch10path, 'legacy': self.legacy}
    
    def __repr__(self):
        # return '{id: {:d}, ch10path: {:s}, legacy: {}'.format(
        #     self.id, self.ch10path, self.legacy
        # )
        return str(self.as_dict())
    
    def fill(self, req_dict):
        if not self.validate_request(req_dict):
            return False

        self.ch10path = req_dict.get('ch10path')
        self.legacy = req_dict.get('legacy')
        return True
        
    