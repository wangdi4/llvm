"""
Simple test framework
"""
import sys, os.path, platform, re, time, traceback, glob, shutil
from datetime import timedelta
from cmdtool import CommandLineTool

class TimeOutException(Exception):
    "Custom timeout exception. Usually raised on task or suite timeouts"
    pass

class EnvironmentValue:
    """
       Simple class to store named environment value. 
       The main usage of this class is to simplify setting the environment variable,
       and then being able to restore its state back 
    """
    def __init__(self, envname):
        self.envname = envname
        if self.envname in os.environ: 
            self.oldvalue = os.environ[self.envname]
            self.value_exist = True
        else:
            self.value_exist = False
    
    def isExisted(self):
        return self.value_exist
    
    def getOldValue(self):
        if self.isExisted():
            return os.environ[self.envname]
        return ''
    
    def setValue(self, value):
        os.environ[self.envname] = value
    
    def appendToFront(self, value, delim = ''):
        if delim == '':
            newVal = value + self.getOldValue()
        elif self.getOldValue() != '':
            newVal = value + delim + self.getOldValue()
        else:
            newVal = value
        self.setValue(newVal)

    def appendToBack(self, value, delim = ''):
        if delim == '':
            newVal = self.getOldValue() + value
        elif self.getOldValue() != '':
            newVal = self.getOldValue() + delim + value  
        else:
            newVal = value
        self.setValue(newVal)
    
    def restoreValue(self):
        if(self.value_exist):
            os.environ[self.envname] = self.oldvalue
        else:
            os.environ.pop(self.envname)

class VSVersion:
    """ 
    Specific Visual Studio version informantion
    """
    def __init__(self, version, programFiles_relpath, cmake_builder_name):
        self.version              = version
        self.programFiles_relpath = programFiles_relpath
        self.cmake_builder_name   = cmake_builder_name

VSVersion_List = { 9  : VSVersion(9,  "Microsoft Visual Studio 9.0",  "Visual Studio 9 2008"),
                   10 : VSVersion(10, "Microsoft Visual Studio 10.0", "Visual Studio 10") }
                   

class VSEnvironment:
    """
    Visual Studio environment
    """
    def __init__(self, version):
        self.vc_version = VSVersion_List[version]

    def DevEnvPath(self):
        return os.path.join(os.environ['ProgramFiles(x86)'] , self.vc_version.programFiles_relpath ,'Common7', 'IDE', 'devenv.com' )
    
    def CMakeGenerator(self):
        return self.vc_version.cmake_builder_name

    def Asm64Path(self):
        path = os.getenv("ProgramFiles(x86)").replace('\\','/')
        return path + '/' + self.vc_version.programFiles_relpath + '/VC/bin/x86_amd64/ml64.exe'

