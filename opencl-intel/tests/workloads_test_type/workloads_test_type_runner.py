import sys
import os
from cmk_test_type_runner import *

def main():
    osType = sys.platform
    #add current working directory to PATH / LD_LIBRARY_PATH in order to find libs
    if osType == 'win32':
        oldPath = os.getenv('PATH')
        os.putenv('PATH', os.getcwd() + ";" + oldPath)
    else:
        oldPath = os.getenv('LD_LIBRARY_PATH')
        if oldPath == None:
            os.putenv('LD_LIBRARY_PATH', os.getcwd())
        else:
            os.putenv('LD_LIBRARY_PATH', os.getcwd() + ":" + oldPath)
    prefixPath = 'validation/workloads_test_type'
    os.chdir(prefixPath)
    prefixPath = '.'
    script = prefixPath + '/cmk_workloads_test_type.py'
    if osType == 'win32':
        cfgPath = prefixPath + '/workloads_benchmarks/cfg.xml'
    else:
        cfgPath = prefixPath + '/workloads_benchmarks/cfg_linux.xml'        
    outDir = prefixPath + '/result'
    
    run_test_type(prefixPath, script, cfgPath, outDir)    
    
main()
