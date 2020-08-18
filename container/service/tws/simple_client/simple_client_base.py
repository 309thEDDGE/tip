#!/usr/bin/env python

import os, sys
from tws.format import *
from abc import ABCMeta, abstractmethod
import json

class SimpleClientBase(object):
    __metaclass__ = ABCMeta

    def __init__(self):
        self.pf = ParseForm()
        self.tf = TranslateForm()
        self.is_parse_job = True
        self.call_types = ['get', 'post']
        self.service_url = os.getenv('SERVICE_URL')
        self.service_port = os.getenv('SERVICE_PORT')

        self.service_defined = False
        if (self.service_url is not None) and (self.service_port is not None):
            self.service_defined = True
        else:
            print('Environment variables not defined!')
        

    def send(self, call_type):
        if call_type == 'get':
            self.make_call(call_type)
        else:
            if self.parse_args():
                self.make_call(call_type)

    @property
    @abstractmethod
    def api_string(self):
        '''
        API call part of URL.
        '''
        pass
        
    @property
    @abstractmethod
    def label_type_dict(self):
        '''
        Defines the argument order of modifiers to the default form dict.
        Example: {'ch10path': str, 'legacy': bool}
        '''
        pass


    def parse_args(self):
        if not self.service_defined:
            return False
        lt_keys = [k for k in self.label_type_dict.keys()]
        if self.is_parse_job:
            default_keys = [k for k in self.pf.default_dict.keys()]
        else:
            default_keys = [k for k in self.tf.default_dict.keys()]
        #print(default_keys)
        for i in range(1, len(sys.argv)):
            currval = sys.argv[i]
            currkey = default_keys[i-1]
            print('arg {:d} is {:s}'.format(i, currval))
            if currkey in lt_keys:
                print('key present in label_type_dict!')
                curr_type = self.label_type_dict[currkey]
                if curr_type == bool:
                    casted_val = bool(int(currval))
                else:
                    casted_val = curr_type(currval)
                if self.is_parse_job:
                    self.pf.default_dict[currkey] = casted_val
                else:
                    self.tf.default_dict[currkey] = casted_val
            else:
                print('{:s} not present in label_type_dict'.format(currkey))
                return False
        if self.is_parse_job:
            print('updated form:', self.pf.default_dict)
        else:
            print('updated form:', self.tf.default_dict)
        return True

    def make_call(self, call_type):
        if self.service_defined is not None:
            if call_type in self.call_types:
                url = 'http://' + self.service_url + ':' + self.service_port + self.api_string
                print('url is {:s}'.format(url))
                if call_type == 'get':
                    commands = ['curl', '-i', url]
                    print(commands)
                    ret = os.system(' '.join(commands))
                    print(ret)
                elif call_type == 'post':
                    json_data = ''
                    if self.is_parse_job:
                        json_data = str(json.dumps(self.pf.default_dict))
                    else:
                        json_data = str(json.dumps(self.tf.default_dict))
                    commands = ['curl', '-i', '-H', '\"Content-Type: application/json\"', 
                                '-X', 'POST', '-d', '\'{:s}\''.format(json_data), url]
                    print(commands)
                    ret = os.system(' '.join(commands))
                    print(ret)
        
