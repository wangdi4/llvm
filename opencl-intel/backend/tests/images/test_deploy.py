'''
Created on Feb 20, 2011

@author: vdovleka
'''
import os
import shutil
import glob

class IOUtils(object):
    def copy(self, src, dest):     
        '''Copy all files matching supplied filename pattern '''
        for file in glob.iglob(src):
            shutil.copy(file, dest)

def main():
    ioutils = IOUtils()
    ioutils.copy(sys.argv[1], sys.argv[2]) 
        
if __name__ == "__main__":
        import sys
        import platform
        if platform.platform().startswith("CYGWIN"):
            print "Cygwin Python is not supported. Please use ActiveState Python."
            sys.exit(1);
        if sys.version_info < (2, 6):
            print "Python version 2.6 or later required"
            sys,exit(1)
        main_result = main()
        sys.exit(main_result)
