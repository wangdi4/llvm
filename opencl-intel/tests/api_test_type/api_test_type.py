import subprocess
import sys
import os
from optparse import OptionParser

# make sure PASS file does not exist
# if PASS file exist, MTV will assume the test completed successfully)
if os.path.isfile('PASS'):
    os.unlink('PASS')

from util.mtv_environment import *    # MTV services such as environment variable access, etc.
from util.mtv_xml_entities import *   # XML parsing and validating services.

#define the test type's executable file name
TEST_TYPE_EXECUTABLE_NAME = 'api_tester.exe'
#define the test type's xml schema file name
TEST_TYPE_SCHEMA_NAME     = 'api_test_type.xsd'

class XmlAPITestCase(XMLEntity):
	def __init__(self, doc_root):
		XMLEntity.__init__(self, doc_root = doc_root)

	def get_name(self):
		return self.get_attribute_by_name('Name')


class XmlAPITest(XMLEntity):
	def __init__(self, cfg, schema):
		XMLEntity.__init__(self, cfg, schema)

	def get_test_cases(self):
		test_cases_list = []
		test_cases = self.get_child_by_name("TestCases")
		for tc in test_cases:
			test_cases_list.append(XmlAPITestCase(doc_root = tc))
		return test_cases_list


def execute_test(exe, test_case_name, output_log, output_err_log):

	# prepare the arguments list
	args = [exe, '-t', test_case_name]

	out = open('res.out','w')

        try:
            subprocess.Popen(args,
                             stdout=out,
                             stderr=output_err_log,
                             shell=True
                            ).wait()
        except:
            print 'subprocess.Popen error'

        out.close()

        out = open('res.out','r')
        test_result = out.read()
        output_log.write(test_result)
        out.close()

        result = 0
        if "All tests succeeded" in test_result:
        	result = 1

	return result

def main():

	# prepare the command line options and acquire them
	# the command's usage should be as follow:
	# "<command> <cfg_file>.xml"
	parser = OptionParser(usage ='usage: %prog <cfg_file>.xml', description = "API test, used as a template and an example for writing test types for MTV.")
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
	data = XmlAPITest(test_config_file, test_schema)

	# now we can use XmlFrameworkTest, for the test type work.
	# instead, we will print the XML file contents:
	#file_contents = file(test_config_file, 'r').readlines()
	#for line in file_contents:
	#    print line

	exp_out_file = open('api_test_type.out', 'a+')
	exp_err_out_file = open('api_test_type.err.out', 'a+')

        test_status = 'PASS'
        result = 1
	for tc in data.get_test_cases():
		print 'Executing: ' + tc.get_name()
		result = execute_test(exe, tc.get_name(), exp_out_file, exp_err_out_file)
		if result is 0:
                    test_status = 'FAIL'

	exp_out_file.close()
	exp_err_out_file.close()

	if test_status is 'PASS':
		print 'Test Passed'
	else:
		print 'Test Failed'

	fd = open(test_status,'w')
	fd.write(test_status)
	fd.close()


if __name__ == '__main__':
    main()
