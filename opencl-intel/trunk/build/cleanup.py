"""
Cleans the environment for the next build
"""
import os, os.path, errno, subprocess, shutil, glob
import logging
import platform
from optparse import OptionParser
import sys

class Config:
    def __init__(self, root_dir, target_type, build_type):
        self.target_type = target_type
        self.build_type  = build_type
        self.root_dir    = root_dir
        self.build_dir    = os.path.join(root_dir, 'build')
        self.system_dir  = os.path.join(self.build_dir, target_type)
    
        if platform.system() == 'Windows':
            self.bin_dir = os.path.join(self.system_dir, 'bin',  build_type)
        else:
            self.system_dir = os.path.join(self.system_dir, build_type)
            self.bin_dir = os.path.join(self.system_dir, 'bin')

def main():
    parser = OptionParser()
    parser.add_option("-r", "--root", dest="root_dir", help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build", dest="build_type", help="build type: Debug, Release", default="Release")
    
    (options, args) = parser.parse_args()

    config = Config(options.root_dir, 
                    options.target_type, 
                    options.build_type)
    
    # clean everything
    if os.path.exists(config.system_dir): 
        print "Cleaning " + config.system_dir
        shutil.rmtree(config.system_dir)

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
