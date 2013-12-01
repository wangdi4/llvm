# Checks SATest will fail with specific error output
# RUN: python %s.py -c %s.cfg

import os, subprocess
from optparse import OptionParser
import sys
import platform

if platform.platform().startswith("CYGWIN"):
    print "Cygwin Python is not supported. Please use ActiveState Python."
    sys.exit(1);
if sys.version_info < (2, 7):
    print "Python version 2.7 or later required"
    sys,exit(1)

parser = OptionParser()
parser.add_option("-c", dest="config",    default=None)

(options, args) = parser.parse_args()
execstr = os.path.join(os.path.abspath(os.getcwd()),"SATest")
confstr = "-config=" + options.config

try:
    # run SATest. it should fail
    subprocess.check_output([execstr,"-OCL","-REF", confstr], stderr=subprocess.STDOUT)
    # if not failed generate exception. test fails.
    raise IOError
except subprocess.CalledProcessError, e:
    # catch failing SATest
    pass

# check output of SATest has warning about OCLBACKEND_PLUGINS variable
e.output.index("workDim # 0 globalWorkSize = 5 is not evenly divisible by localWorkSize = 3")