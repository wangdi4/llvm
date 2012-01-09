"""
  This file is a template for creating test types in MTV.

  This file can be executed from within a valid MTV shell.

  This file takes an XML name file from the command line
and an XSD schema from the shell IMPORT directory (the
schema file name is given hard coded).
  It then parses the XML file, validates it against the
schema, and prints the XML file to the screen.


"""

import subprocess
import sys
import os
import shutil
from optparse import OptionParser

# make sure PASS file does not exist
# if PASS file exist, MTV will assume the test completed successfully)
if os.path.isfile('PASS'):
    os.unlink('PASS')

from util.mtv_environment import *    # MTV services such as environment variable access, etc.
from util.mtv_xml_entities import *   # XML parsing and validating services.

#define the test type's executable file name
TEST_TYPE_EXECUTABLE_NAME = 'framework_test_type.exe'
#define the test type's xml schema file name
TEST_TYPE_SCHEMA_NAME     = 'framework_test_type.xsd'

class XmlFrameworkTestCase(XMLEntity):
	"""
	this class implements the test-type specific requirements from its XML file.
	It inherits from XMLEntity that gives access to basic methods such as
	parsing and validating against a schema, and gives access to elements by
	their name (only to the root element's children. if further levels are
	needed, you need to implement another class - example in xml_wrapper_test_type)
	"""
	def __init__(self, doc_root):
		XMLEntity.__init__(self, doc_root = doc_root)

	def get_name(self):
		return self.get_attribute_by_name('Name')


class XmlFrameworkTest(XMLEntity):
	"""
	this class implements the test-type specific requirements from its XML file.
	It inherits from XMLEntity that gives access to basic methods such as
	parsing and validating against a schema, and gives access to elements by
	their name (only to the root element's children. if further levels are
	needed, you need to implement another class - example in xml_wrapper_test_type)
	"""
	def __init__(self, cfg, schema):
		XMLEntity.__init__(self, cfg, schema)


	def get_test_cases(self):
		test_cases_list = []
		test_cases = self.get_child_by_name("TestCases")
		for tc in test_cases:
			test_cases_list.append(XmlFrameworkTestCase(doc_root = tc))
		return test_cases_list


def execute_test(exe, test_case_name):

	# prepare the arguments list
	args = [exe, test_case_name]

	exp_out = 'result_' + test_case_name + '.out'
	exp_out_file = open(exp_out,'w')

	exp_err_out = 'result_err_' + test_case_name + '.out'
	exp_err_out_file = open(exp_err_out,'w')

        try:
            subprocess.Popen(args,
                             stdout=exp_out_file,
                             stderr=exp_err_out_file,
                             shell=True
                            ).wait()
        except:
            print 'subprocess.Popen error'
                     
	exp_out_file.close()
	exp_err_out_file.close()

	return exp_out, exp_err_out

def main():

	# prepare the command line options and acquire them
	# the command's usage should be as follow:
	# "<command> <cfg_file>.xml"
	parser = OptionParser(usage ='usage: %prog <cfg_file>.xml', description = "Framework test, used as a template and an example for writing test types for MTV.")
	(options, args) = parser.parse_args()

	# check that we got a task file from the user, abort if not
	if len(args) is not 1:
		parser.error("Illegal Execution Command line!")

	# get local environment variables
	mtv_env = MTVEnvironment()

	# make sure the configuration file, we got as parameter, exists
	test_config_file = os.path.abspath(args[0])
	if not os.path.isfile(test_config_file):
		sys.exit('Could not find configuration file, test failed')

	# make sure the executable of the test type exist
	# FOR DEMONSTRATION PURPOSE, WE WILL SEARCH FOR THIS FILE ONLY IF
	# THE VARIABLE TEST_TYPE_EXECUTABLE_NAME CONTAINS SOMETHING:
	if TEST_TYPE_EXECUTABLE_NAME is not '':
		exe = mtv_env.search_for_file_in_bin_dir(TEST_TYPE_EXECUTABLE_NAME)
		print exe
		if not os.path.isfile(exe):
			sys.exit('Could not find the test type executable file, test failed')

	# get the required schema files
	test_schema = mtv_env.search_for_file_in_import_dir(TEST_TYPE_SCHEMA_NAME)
	if not test_schema:
		sys.exit('Could not find the test type schema file, test failed')


	# here we perform the XML parsing and validating.
	# if the validation fails, an exception is thrown.
	# you can catch this by using 'try-except' commands.
	data = XmlFrameworkTest(test_config_file, test_schema)


        #copy binary files to run dir
        shutil.copy(os.environ['MTV_LOCAL_BIN_DIR'] + "\\test.bc", ".\\test.bc")

	# now we can use XmlFrameworkTest, for the test type work.
	# instead, we will print the XML file contents:
	#file_contents = file(test_config_file, 'r').readlines()
	#for line in file_contents:
	#    print line

	os.environ['CL_CONFIG_LOG_FILE'] = os.path.join(os.getcwd(), 'framework_test_type_cl.log')

	test_status = 'FAIL'
	for tc in data.get_test_cases():
		print 'Executing: ' + tc.get_name()
		test_status = 'FAIL'
		out, err = execute_test(exe, tc.get_name())
	# we can check the out and the err files to decide result status
		test_result = open(out).read()
		if "TEST SUCCEDDED" in test_result:
			test_status = 'PASS'
		else:
			break

	# now, collect all log files and put the data in single file

        final_log_out = open('framework_test_type.out', 'w')
	for tc in data.get_test_cases():
            single_out = 'result_' + tc.get_name() + '.out'
            single_out_file = open(single_out,'r')
            final_log_out.write(single_out + "\n" + single_out_file.read() + "\n")
            single_out_file.close()
        final_log_out.close()
                        
	if test_status is 'PASS':
		print 'Test Passed'
	else:
		print 'Test Failed'

	fd = open(test_status,'w')
	fd.write(test_result)
	fd.close()


if __name__ == '__main__':
    main()
