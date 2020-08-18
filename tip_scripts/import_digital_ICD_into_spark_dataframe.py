import pickle

def import_digital_icd(spark, icd_path):
    
    df = spark.read.csv(icd_path, header=True)
    return df

def pickle_dict(file_path, thedict):
    with open(file_path, 'wb') as f:
        pickle.dump(thedict, f, protocol=pickle.HIGHEST_PROTOCOL)

def read_pickled_dict(file_path):
    thedict = {}
    with open(file_path, 'rb') as f:
        thedict = pickle.load(f)
    return thedict

#def create_pickle_path(other_path):
