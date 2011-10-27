'''
Created on Nov 23, 2010

@author: myatsina
'''
from optparse import OptionParser
import os
import ConfigParser
from BackEndValidation import Test, ValidatorError

class ExtendedConfigParser(ConfigParser.SafeConfigParser):
    '''
    This class modifies behavior of get(), by adding the following rule:
    When searching for a key 'key1' in section 'section1' under target 'target1'
    The following is 
    [section1]
    key1_target1: SomeValue1 // This is the key with highest priority
    key1: SomeValue2         // This is the key with lower priority
    key1_SomeOtherTarget: SomeValue3 // Ignored
    '''
    def __init__(self, target_platform):
        ConfigParser.SafeConfigParser.__init__(self)
        self.target_platform = target_platform
    
    def getTargetSpecificKey(self, key):
        return key + '_' + self.target_platform

    def get(self, section, key):
        '''
        Reads config key from config file.
        First searches for $Key_$TargetPlatform.
        If not found, searches for $Key.
        '''	

        TSK = self.getTargetSpecificKey(key)
        if ConfigParser.SafeConfigParser.has_option(self, section, TSK):
            return ConfigParser.SafeConfigParser.get(self, section, TSK)
        return ConfigParser.SafeConfigParser.get(self, section, key)

    def has_option(self, section, key):
        return ConfigParser.SafeConfigParser.has_option(self, section, self.getTargetSpecificKey(key)) or ConfigParser.SafeConfigParser.has_option(self, section, key)


class ValidatorOptionsParser(object):
    '''
    classdocs
    '''


    def __init__(self):
        '''
        Constructor
        
        Creates the program options
        '''
        
        self.parser = OptionParser()
        
        self.TARGET_PLATFORM = ['Win32', 'Win64', 'Linux64']
        
        self.parser.add_option('-c', '--config', dest='config', help='Configuration file, default: tests_nightly.cfg', default='tests_nightly.cfg')
        self.parser.add_option('-r', '--run-only', dest='runOnly', help='Run only, don\'t copy binaries, default: False', action='store_true')
        self.parser.add_option('-v', '--validate-only', dest='validateOnly', help='Validate output logs only, don\'t copy binaries and don\' run tests, default: False', action='store_true')
        self.parser.add_option('-n', '--no-import', dest='noImport', help='Don\'t copy OpenCL SDK binaries directory, default: False', action='store_true')
        self.parser.add_option('-l', '--log', dest='logFile', help='Log file')
        self.parser.add_option('-o', '--out', dest='outputDir', help='Output directory')
        self.parser.add_option('-s', '--src', dest='importBinSrcDir', help='OpenCL SDK binaries source directory, specifies from where to copy OpenCL SDK binaries')
        self.parser.add_option('-d', '--dest', dest='importBinDestDir', help='OpenCL SDK binaries destination directory, specifies to where to copy OpenCL SDK binaries')
        self.parser.add_option('-w', '--work-dir', dest='workingDir', help='Working directory from where to run the tests')
        self.parser.add_option('-b', '--backend', dest='oclCpuBackEnd', help='OclCpuBackEnd DLL location')
        self.parser.add_option('-f', '--func', dest='builinFunctions', help='Built in functions DLLs and RTLs location')
        self.parser.add_option('-m', '--svml', dest='svml', help='SVML DLLs location')
        self.parser.add_option('-t', '--target-platform', dest='target_platform', default=self.TARGET_PLATFORM[0], help='Target platform. One of ' + str(self.TARGET_PLATFORM) + '. Default: ' + self.TARGET_PLATFORM[0])

    def parse(self):
        '''
        Parses options
        
        If options are not specified in the command line their configuraiton file value is used
        '''
        
        options = self.parser.parse_args()[0]
        
        self.target_platform = options.target_platform
        if self.target_platform not in self.TARGET_PLATFORM:
            raise ValidatorError.IlegalArgument('Wrong target platform was set: ' + self.target_platform + '. Allowed platforms: ' + str(self.TARGET_PLATFORM))    

        config = ExtendedConfigParser(self.target_platform)
        config.read(options.config)
        
        config.set('General', 'configuration', self.target_platform)
        config.set('General', 'file_protocol', config.get('General', 'protocol'))
        
        self.runOnly = options.runOnly
        self.validateOnly = options.validateOnly 
        self.noImport = options.noImport
        
        self.logFile = options.logFile
        self.outputDir = options.outputDir
        self.importBinSrcDir = options.importBinSrcDir 
        self.importBinDestDir = options.importBinDestDir
        self.workingDir = options.workingDir
        self.oclCpuBackEnd = options.oclCpuBackEnd
        self.builinFunctions = options.builinFunctions
        self.svml = options.svml

        # Now it is safe to call getConfig()

        if self.logFile is None:
            self.logFile = config.get('General', 'logFile')
        self.logFile = os.path.normpath(self.logFile)
            
        if self.outputDir is None:
            self.outputDir = os.path.abspath(config.get('General', 'outputDir'))
        self.outputDir = os.path.normpath(self.outputDir)
    
        if self.importBinSrcDir is None:
            self.importBinSrcDir = config.get('General', 'importBinSrcDir')
        self.importBinSrcDir = os.path.normpath(self.importBinSrcDir)
        
        if self.importBinDestDir is None:
            self.importBinDestDir = config.get('General', 'importBinDestDir')
        self.importBinDestDir = os.path.normpath(self.importBinDestDir)
        
        if self.workingDir is None:
            self.workingDir = os.path.join(self.importBinDestDir, config.get('General', 'relativeWorkingDir'))
        self.workingDir = os.path.normpath(self.workingDir)
        
        if self.oclCpuBackEnd is None:
            self.oclCpuBackEnd = config.get('General', 'oclCpuBackEnd')
        self.oclCpuBackEnd = os.path.normpath(self.oclCpuBackEnd)
            
        if self.builinFunctions is None:
            self.builinFunctions = config.get('General', 'builinFunctions')
        self.builinFunctions = os.path.normpath(self.builinFunctions)
            
        if self.svml is None:
            self.svml = config.get('General', 'svml')
        self.svml = os.path.normpath(self.svml)       

        self.tests = self.getTestsFromConfig(config, self.target_platform)
        
    def isTestForMe(self, test_name, my_target_platform):

        for p in self.TARGET_PLATFORM:
            if test_name.endswith('.' + p):
                if p == my_target_platform:
                    return True
                return False

        return True
    

    def getTestsFromConfig(self, config, target_platform):
        '''
        Creates test object based on the test written in the configuration file.
        
        @param config: Configuraiton parser that allows to retrieve tests information 
        @type config: ConfigParser
        
        @return: List of tests to run
        @rtype: Test List
        '''
        tests = []
        for section in config.sections():
            if section.startswith('Test.') and self.isTestForMe(section, target_platform):
                test = Test.Test(section, self.outputDir, config)
                tests.append(test)
           
        return tests
    
    
    def getRunOnly(self):
        '''        
        @return: True denotes only run tests, do not import binaries and DLLs. 
        @rtype: Boolean
        '''
        return self.runOnly
              
    def getValidateOnly(self):
        '''
        @return: True denotes only validate test output logs, do not copy binaries and DLLs, do not run tests. 
        @rtype: Boolean
        '''
        return self.validateOnly
    
    def getNoImport(self):
        '''        
        @return: True denotes do not import OpenCL SDK binaries, but do copy OclCpuBackEnd, built-in functions and SVML DLLs. 
        @rtype: Boolean
        '''
        return self.noImport
        
    def getLogFile(self):
        '''
        @return: Name of OclCpuBackEnd log file
        @rtype: String
        '''
        return self.logFile
        
    def getOutputDir(self):
        '''
        @return: Name of output directory
        @rtype: String
        '''
        return self.outputDir
        
    def getImportBinSrcDir(self):
        '''
        @return: Name of the source binary directory that needs to be imported (from where to copy the OpenCL and WOLF SDK binaries)
        @rtype: String
        '''
        return self.importBinSrcDir
        
    def getImportBinDestDir(self):
        '''
        @return: Name of the destination binary directory that needs to be imported (to where to copy the OpenCL and WOLF SDK binaries)
        @rtype: String
        '''
        return self.importBinDestDir
        
    def getWorkingDir(self):
        '''
        @return: Name of working directory (this directory is always relative to importBinDestDir)
        @rtype: String
        '''
        return self.workingDir
        
    def getOclCpuBackEnd(self):
        '''
        @return: Name of OclCpuBackEnd DLL directory
        @rtype: String
        '''
        return self.oclCpuBackEnd
        
    def getBuilinFunctions(self):
        '''
        @return: Name of built-in functions DLLs and RTLs directory
        @rtype: String
        '''
        return self.builinFunctions
        
    def getSVML(self):
        '''
        @return: Name of SVML DLLs directory
        @rtype: String
        '''
        return self.svml

    def getTests(self):
        '''
        @return: List of tests to run
        @rtype: Test List
        '''
        return self.tests
