# Test creates environment variable for enablling SATest OCL Recorder
# Checks SATest will fail with specific error output
# RUN: python OclBackendPluginsSet %s.py -c=%s.cfg -d=%t
import os, subprocess
from optparse import OptionParser

parser = OptionParser()
parser.add_option("-c", dest="config",    default=None)
parser.add_option("-d", dest="llvm_file",    default=None)
(options, args) = parser.parse_args()

try:
	# set recorder enabling environment variable
	os.environ["OCLBACKEND_PLUGINS"]="xxx.xxx"
	execstr = 'SATest -OCL -VAL' + ' -config=' + options.config + ' -dump-llvm-file=' + options.llvm_file
	# run SATest. it should fail
	subprocess.check_output(execstr, stderr=subprocess.STDOUT)
	# if not failed generate exception. test fails.
	raise IOError
except subprocess.CalledProcessError, e:
	# catch failing SATest
	pass
# check output of SATest has warning about OCLBACKEND_PLUGINS variable
e.output.index("variable OCLBACKEND_PLUGINS exists")
