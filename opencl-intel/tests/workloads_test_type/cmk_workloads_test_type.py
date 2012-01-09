"""
Workloads test type running script
"""

import subprocess
import sys
import os
import shutil
from optparse import OptionParser

# make sure PASS file does not exist
# if PASS file exist, MTV will assume the test completed successfully)
if os.path.isfile(os.getenv('OUTPUT_DIRECTORY') + '/PASS'):
    os.unlink(os.getenv('OUTPUT_DIRECTORY') + '/PASS')

from cmk_xml_entities import *   # XML parsing and validating services.

if sys.platform == 'win32':
    exeSuffix = '.exe'
else:
    exeSuffix = ''

#define the test type's executable file name
TEST_TYPE_EXECUTABLE_NAME = os.getenv('EXE_DIRECTORY') + '/workloads_test_type' + exeSuffix
#define the test type's xml schema file name
TEST_TYPE_SCHEMA_NAME     = os.getenv('EXE_DIRECTORY') + '/workloads_test_type.xsd'
#define the test type's output file name
TEST_TYPE_LOG_FILE_NAME   = 'workloads_test_type.out'

class XmlWorkload(XMLEntity):
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
	def get_iterations_count(self):
		return self.get_attribute_by_name('Iterations')
	def get_configuration(self):
		return self.get_attribute_by_name('Configuration')
	def get_file_to_copy(self):
		return self.get_attribute_by_name('FileToCopy')
	    


class XmlWorkloadsConfig(XMLEntity):
	"""
	this class implements the test-type specific requirements from its XML file.
	It inherits from XMLEntity that gives access to basic methods such as
	parsing and validating against a schema, and gives access to elements by
	their name (only to the root element's children. if further levels are
	needed, you need to implement another class - example in xml_wrapper_test_type)
	"""
	def __init__(self, cfg, schema):
		XMLEntity.__init__(self, cfg, schema)

        def get_output_file_name(self):
            output_file_name = self.get_child_text_by_name("OutputFile")
            return output_file_name

	def get_workloads(self):
		test_cases_list = []
		test_cases = self.get_child_by_name("Workloads")
		print test_cases
		for tc in test_cases:
			test_cases_list.append(XmlWorkload(doc_root = tc))
		return test_cases_list


def execute_test(exe, output_file_name, workload_name, workload_configuration):

        outputDirectory = os.getenv('OUTPUT_DIRECTORY')

	# prepare the arguments list
	if sys.platform == 'win32':
            args = [os.path.normpath(exe), outputDirectory + '/' + output_file_name, workload_name, '1', workload_configuration]
        else:
            args = './' + os.path.normpath(exe) + " " + outputDirectory + '/' + output_file_name + " " + workload_name + " " +  '1' + " " +  workload_configuration

	print args
        
	exp_out = outputDirectory + '/result_' + workload_name + '.out'
	exp_out_file = open(exp_out,'w')

	exp_err_out = outputDirectory + '/result_err_' + workload_name + '.out'
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
	parser = OptionParser(usage ='usage: %prog <cfg_file>.xml', description = "Workloads test.")
	(options, args) = parser.parse_args()

	# check that we got a task file from the user, abort if not
	if len(args) is not 1:
		parser.error("Illegal Execution Command line!")

	# get local environment variables

	# make sure the configuration file, we got as parameter, exists
	test_config_file = os.path.abspath(args[0])
	if not os.path.isfile(test_config_file):
		sys.exit('Could not find configuration file, test failed')

	# make sure the executable of the test type exist
	if TEST_TYPE_EXECUTABLE_NAME is not '':
		exe = TEST_TYPE_EXECUTABLE_NAME
		print exe
		if not os.path.isfile(exe):
			sys.exit('Could not find the test type executable file, test failed')

	# get the required schema files
	test_schema = TEST_TYPE_SCHEMA_NAME
	if not test_schema:
		sys.exit('Could not find the test type schema file, test failed')


	# here we perform the XML parsing and validating.
	# if the validation fails, an exception is thrown.
	# you can catch this by using 'try-except' commands.
	data = XmlWorkloadsConfig(test_config_file, test_schema)

	# now we can use XmlFrameworkTest, for the test type work.
	# instead, we will print the XML file contents:
	#file_contents = file(test_config_file, 'r').readlines()
	#for line in file_contents:
	#    print line

	os.environ['CL_CONFIG_LOG_FILE'] = os.path.join(os.getcwd(), 'workloads_test_type.log')

	test_status = 'FAIL'
	for workload in data.get_workloads():
            print 'Executing: ' + workload.get_name()
            test_status = 'FAIL'
            out, err = execute_test(exe, data.get_output_file_name(), workload.get_name(), workload.get_configuration())

            # we can check the out and the err files to decide result status
            test_result = open(out).read()
            if "TEST SUCCEDDED" in test_result:
                test_status = 'PASS'
            else:
                break

	if test_status is 'PASS':
            print 'Test Passed'
	else:
            print 'Test Failed'

        outputDirectory = os.getenv('OUTPUT_DIRECTORY')

	fd = open(outputDirectory + '/' + test_status,'w')
	fd.write(test_result)
	fd.close()


if __name__ == '__main__':
    main()
