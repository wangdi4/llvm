'''
Created on Feb 20, 2011

@author: myatsina
'''
import os
import shutil
from BackEndValidation import ValidatorError
import random
import time

class IOManager(object):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        '''
        self.NUM_RETRIES = 5
        self.MIN_RETRY_PERIOD = 0           # in secods
        self.MAX_RETRY_PERIOD = 10          # in secods
        
        random.seed()

    
    def safeIO(self, errorMsg, func, *args):     
        '''
        Creates new Validator.
        
        @param path: Deletes specified path tree if it exists.
        @type path: String 
        '''
        
        retries = 0
        
        while retries < self.NUM_RETRIES :
            
            try:
                
                return func(*args)
                    
            except Exception as origException:
                retries = retries + 1
                time.sleep(random.randint(self.MIN_RETRY_PERIOD, self.MAX_RETRY_PERIOD))
        
        raise ValidatorError.IOError(errorMsg, origException)
    
    def delete(self, path, rmTree = True):     
        '''
        Creates new Validator.
        
        @param path: Deletes specified path tree if it exists.
        @type path: String 
        '''  
        
        if not os.path.exists(path):
            return
        
        if rmTree:
            func = shutil.rmtree
        else:
            func = os.remove
            
        self.safeIO('Error deleting path ' + path, func, path)
        
    def copy(self, src, dest, copyTree = True):     
        '''
        Creates new Validator.
        
        @param path: Deletes specified path tree if it exists.
        @type path: String 
        '''
        
        if copyTree:
            func = shutil.copytree
        else:
            func = shutil.copy
        
        self.safeIO('Error copying from ' + src + ' to ' + dest, func, src, dest)
        
    
    def openFile(self, filePath, permissions):
        return self.safeIO('Could not open file ' + filePath, open, filePath, permissions)
    
    def makedirs(self, dirPath):
        self.safeIO('Could not create directory path ' + dirPath, os.makedirs, dirPath)
        
