from flask import Flask
from flask_sqlalchemy import SQLAlchemy
#from flask_migrate import Migrate
from config import Config

# def create_app():
app = Flask(__name__)
app.config.from_object(Config)
    
#migrate = Migrate(app, db)
db = SQLAlchemy(app)
import tws.models
db.create_all()
from tws.views import tip_control
app.register_blueprint(tip_control)
    
    # return app
