
import os
db_dir = os.path.abspath(os.path.dirname(__file__))

class Config:
    SQLALCHEMY_DATABASE_URI = os.environ.get('DATABASE_URL') or \
        'sqlite:///' + os.path.join(db_dir, 'tws.db')
    SQLALCHEMY_TRACK_MODIFICATIONS = False