/*
 * NoInputArgsTestRunner.h
 *
 *  Created on: Jun 11, 2009
 *      Author: openCL
 */

#ifndef TESTRUNNER_H_
#define TESTRUNNER_H_

#include <string>
#include <vector>
#include <set>
#include <list>
#include <exception>

#include "TestFailedException.h"
#include "../KernelExecutor/Execution.h"
#include "../KernelExecutor/RegularParameter.h"
#include "../RandomInputGenerator/RandomInputGenerator.h"

using std::string;
using std::vector;
using std::set;
using std::list;

class TestsRunner {

public:
	TestsRunner();
	virtual ~TestsRunner();
	static void setTestList(const set<string>& testsToRun);
    static void setIgnoreTestList(const set<string>& testsToIgnore);
	int RunAllTests();

private:

	typedef struct {
		int total;
		int succeeded;
		int failed;
		int ignored;
	} TestResults;

	void runTestsInDir(const string & dirName, TestResults & results);
	bool RunTest(const string & testName, const string & path);
	void execute(const string & testName, const string & path, const vector<
			float*> & inputArray, float **outputArray, const list<
			RegularParameter> & inputArgs, const Execution::Type executionType);
	static void setIgnoreList();
	void clearInputs(vector<float*> & normalInputs, vector<float*> & vectorizedInputs);
    void handleException(const string & testName, const string & path,
			exception& e, vector<float*> normalInputs,
			vector<float*> vectorizedInputs);
	list<RegularParameter> getArguments(const string& path);
	bool isInTestsTiRun(string testName, string path);
	bool isInTestsToRun(string testName, string path);

	static const unsigned int NUM_INSTANCES;
	static const unsigned int OUTPUT_SIZE;
	static const unsigned int INPUT_SIZE;
	static const string TESTS_BASE_DIR;
	static set<string> ignoreList;
	static set<string> testsToRun;


};

#endif /* TESTRUNNER_H_ */
