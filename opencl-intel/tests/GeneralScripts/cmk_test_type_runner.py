import os
import subprocess
import sys
import errno
import shutil
import string
import time

def run_test_type(prefixPath, script, cfgPath, outDir):

    if not os.path.isfile(script):
        sys.exit('Could not find the script ' + script)
    if not os.path.isfile(cfgPath):
        sys.exit('Could not find the configuration file')
    if not os.path.isdir(outDir):
        try:
            os.mkdir(outDir)
        except OSError, exc:
            sys.exit('Error occured while trying to create output directoory, Exiting')
            

    outDir = outDir + '/' + time.strftime("%a_%d_%b_%Y_%H_%M_%S", time.localtime())
    try:
        os.mkdir(outDir)
    except OSError, exc:
        sys.exit('Error occured while trying to create output directoory, Exiting')
        
    shutil.copyfile(cfgPath, outDir + '/cfg.xml')
    os.putenv('OUTPUT_DIRECTORY',outDir)
    os.putenv('EXE_DIRECTORY', prefixPath)
    # run external process
    if sys.platform == 'win32':
        args = ["python "+ os.path.normpath(script), cfgPath]
    else:
        args = "python " + os.path.normpath(script) + " " + cfgPath
    process = subprocess.Popen(args , shell=True)
    err = process.wait()

