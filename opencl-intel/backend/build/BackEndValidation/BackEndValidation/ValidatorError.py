'''
Created on Feb 20, 2011

@author: myatsina
'''

class ValidatorError(Exception):
    '''
    classdocs
    '''


    def __init__(self, msg, origException = None):
        '''
        Constructor
        '''
        self.msg = msg
        self.origException = origException
    
    def __str__(self):
        return self.msg
    
    def getOrigException(self):
        return self.origException


class IOError(ValidatorError):
    '''
    classdocs
    '''
    
    
class IlegalArgument(ValidatorError):
    '''
    classdocs
    '''

    