
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <fstream>
using namespace std;


 CPPUNIT_PLUGIN_IMPLEMENT();

int main(int argc, char* argv[])
{
	// Define the file that will store the XML output.
	ofstream outputFile("c:\\outputFile.xml");

	// Get the top level suite from the registry
	CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	// Adds the test to the list of test to run
	CppUnit::TextUi::TestRunner runner;
	runner.addTest( suite );

	// Specify XML output and inform the test runner of this format.
	CppUnit::XmlOutputter* outputter = new CppUnit::XmlOutputter(&runner.result(), outputFile);
	outputter->setStyleSheet("ocl_report.xsl");
	runner.setOutputter(outputter);
	runner.run();
	outputFile.close();

	// Change the default outputter to a compiler error format outputter
	runner.setOutputter( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );
	// Run the tests.
	bool wasSucessful = runner.run();

	// Return error code 1 if the one of test failed.
	return wasSucessful ? 0 : 1;

}