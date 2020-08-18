from tws import db
from tws.models.base_model import BaseModel

class TranslateJob(BaseModel, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    legacy = db.Column(db.Boolean, index=False, unique=False)
    pqpath = db.Column(db.String, index=False, unique=False)
    icdpath = db.Column(db.String, index=False, unique=False)
    nthread = db.Column(db.Integer, index=False, unique=False)

    def __init__(self):
        BaseModel.__init__(self)
        #print('attribs:', self.attribs)
      
    def as_dict(self):
        return {'pqpath': self.pqpath, 'icdpath': self.pqpath, \
                'nthread': self.nthread, 'legacy': self.legacy}
                                                
    def __repr__(self):
        # return '{id: {:d}, pq10path: {:s}, icdpath: {:s}, \
        #         nthread: {:d}'.format(
        #             self.id, self.pqpath, self.icdpath, self.nthread)
        return str(self.as_dict())

    def fill(self, req_dict):
        if not self.validate_request(req_dict):
            return False

        self.pqpath = req_dict.get('pqpath')
        self.icdpath = req_dict.get('icdpath')
        self.nthread = req_dict.get('nthread')
        self.legacy = req_dict.get('legacy')
        return True