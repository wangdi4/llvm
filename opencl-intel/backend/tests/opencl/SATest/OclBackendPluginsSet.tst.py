# Test creates environment variable for enablling SATest OCL Recorder
# Checks SATest will fail with specific error output
# RUN: python OclBackendPluginsSet %s.py -b SATest -c=%s.cfg -d=%t
from __future__ import print_function
import os, subprocess
from optparse import OptionParser
import sys
import platform

if platform.platform().startswith("CYGWIN"):
    print("Cygwin Python is not supported. Please use ActiveState Python.")
    sys.exit(1);
if sys.version_info < (2, 7):
    print("Python version 2.7 or later required")
    sys,exit(1)

parser = OptionParser()
parser.add_option("-c", dest="config",    default=None)
parser.add_option("-d", dest="llvm_file",    default=None)
parser.add_option("-b", dest="binary",    default=None)
(options, args) = parser.parse_args()

execstr = options.binary
confstr = "-config=" + options.config
dumpstr = "-dump-llvm-file=" + options.llvm_file

try:
    # set recorder enabling environment variable
    os.environ["OCLBACKEND_PLUGINS"]="xxx.xxx"
    subprocess.check_output([execstr,"-VAL", confstr,dumpstr], stderr=subprocess.STDOUT)
    # run SATest. it should fail
    subprocess.check_output(execstr, stderr=subprocess.STDOUT)
    # if not failed generate exception. test fails.
    raise IOError
except subprocess.CalledProcessError as e:
    # catch failing SATest
    # check output of SATest has warning about OCLBACKEND_PLUGINS variable
    e.output.decode().index("variable OCLBACKEND_PLUGINS exists")
