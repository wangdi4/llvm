'''
Created on Nov 23, 2010

@author: myatsina
'''
import re
import os

class Test(object):
    '''
    classdocs
    '''



    def getOptionDefault(self, section, config, option, defaultValue):
        if config.has_option(section, option):
            configOption = config.get(section, option)
        else:
            configOption = defaultValue
        return configOption

    def __init__(self, test, outputDir, config):
        '''
        Constructor 
        '''
        self.command = config.get(test, 'command')
        self.logName = os.path.join(outputDir, config.get(test, 'logName'))
        self.timeout = int(config.get(test, 'timeout'))
        self.expectFail = bool(self.getOptionDefault(test, config, 'expectFail', '')) # Empty string '' will translate to False bool
        self.containsCase = self.getOptionalOption(config, test, 'containsCase')
        self.doesntContainNoCase = self.getOptionalOption(config, test, 'doesntContainNoCase')
        self.exceptCase = self.getOptionalOption(config, test, 'exceptCase')
        self.exceptMultiLineCase = self.getOptionalOption(config, test, 'exceptMultiLineCase')
        
    
    def getOptionalOption(self, config, section, option):
        options = []
        if config.has_option(section, option):
            configOption = config.get(section, option)
            
            removeComments = re.split(';.+', configOption)
            
            for elem in removeComments:
                options.extend(re.split('[\t\n\r]+', elem))           
    
        options = [x.strip() for x in options if x != '']
        return options
    
    def getCommand(self):
        return self.command
    
    def getLogName(self):
        return self.logName
    
    def getTimeout(self):
        return self.timeout
    
    def getExpectFail(self):
        return self.expectFail
    
    def getContainsCase(self):
        return self.containsCase
    
    def getDoesntContainNoCase(self):
        return self.doesntContainNoCase
    
    def getExceptCase(self):
        return self.exceptCase
    
    def getExceptMultiLineCase(self):
        return self.exceptMultiLineCase
