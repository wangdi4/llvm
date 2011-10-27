/*
 * NoInputArgsTestRunner.cpp
 *
 *  Created on: Jun 11, 2009
 *      Author: openCL
 */

#include "TestsRunner.h"
#include <atlconv.h>
#include <iostream>
#include <exception>
#include <cmath>
#include <time.h>

#ifdef WINDOWS
#include <windows.h>
#include <float.h>
#else
#include <dirent.h>
#endif

#include "../KernelExecutor/KerenelExecutor.h"
#include "../KernelExecutor/IllegalParameterException.h"
#include "../Utils/StringConvertor.h"

using std::cout;
using std::endl;
using std::exception;
#ifdef WINDOWS
#define IS_NAN _isnan
#else
#define IS_NAN std::isnan
#endif

// initialize static const members


/*
 * number instances that will run during the kernel execution, should not be smaller than 4
 */
const unsigned int TestsRunner::NUM_INSTANCES = 4;

/*
 * size of output vector
 */
const unsigned int TestsRunner::OUTPUT_SIZE = NUM_INSTANCES * 2 * 16; // up to double16

/*
 * size of input vector
 */
const unsigned int TestsRunner::INPUT_SIZE = OUTPUT_SIZE; // up to double 16

/*
 * directory containing all tests
 */
#ifdef WINDOWS
//const string TestsRunner::TESTS_BASE_DIR = "C:/Users/ikleine1/Desktop/CVCC/OCL_VEC_february/tests/OCL_Vectorizer_Tests/tests/src/VectorizerTests/Tests";
//const string TestsRunner::TESTS_BASE_DIR = "src/VectorizerTests/Tests";
const string  TestsRunner::TESTS_BASE_DIR ="tests/OCL_Vectorizer_Tests/tests/src/VectorizerTests/Tests";

#else
const string TestsRunner::TESTS_BASE_DIR = "src/VectorizerTests/Tests";
#endif

set<string> TestsRunner::ignoreList;
set<string> TestsRunner::testsToRun;

TestsRunner::TestsRunner() {
	TestsRunner::setIgnoreList();
}

TestsRunner::~TestsRunner() {
}

void TestsRunner::setTestList(const set<string>& testsToRun) {
	TestsRunner::testsToRun = testsToRun;
}
void TestsRunner::setIgnoreTestList(const set<string>& testsToIgnore) {

        TestsRunner::ignoreList = testsToIgnore;
        for (set<string>::iterator it = ignoreList.begin();it != ignoreList.end();it++)    {
            string stringToIgnore = *it;
            ignoreList.insert(stringToIgnore);
        }

}

/*
 * constructs the list of tests to be ignored.
 */
void TestsRunner::setIgnoreList() {

	    //ignoreList.insert("cf_17_levels"); //infinite time execution
        //ignoreList.insert("func_bitselect"); //bug opened Duplicate of bug CSSD100005611.   Original bug CSSD100005535
        //ignoreList.insert("extractelement_short4"); //bug opened bug CSSD100005611        
        //ignoreList.insert("while_test");  //open bug CSSD100005935
		//ignoreList.insert("if_else");  // assertiom need to open bug    
        //ignoreList.insert("getelementptr_add");  solved
        //ignoreList.insert("if_else_if_else_while");




       // ignoreList.insert("func_clz");      the body function is fixed    
      //  ignoreList.insert("cf_21_depend_id");    //vectorized version never finish.
       // ignoreList.insert("cf_4_level_depend2"); //open bug CSSD100005651.
        //ignoreList.insert("cf_8_level_depend_id");  // open bug CSSD100005650

        //ignoreList.insert("cf_1_level_depend");     //View  Defect  CSSD100005655
        //ignoreList.insert("cf_10_levels_dep3");     //View  Defect  CSSD100005655
        //ignoreList.insert("cf_1_level_depend1");     //View  Defect  CSSD100005655
       // ignoreList.insert("cf_20_levels");  // the Defect CSSD100005651
       // ignoreList.insert("cf_10_levels_notdep");  // the Defect CSSD100005651

       // ignoreList.insert("do_while_switch");  // the Defect CSSD100005726
        //ignoreList.insert("irreducible2");  //CSSD100005730

        //ignoreList.insert("irreducible_without_goto"); // performance gap opened bug CSSD100005731
       // ignoreList.insert("irreducible_without_goto1"); //CSSD100005732 performance + diff result bug
        //ignoreList.insert("irreducible5"); //never stop probabli need to open bug
        //ignoreList.insert("irruducible4"); //never stop probabli need to open bug 1298027140
       // ignoreList.insert("a_irreducible9"); //never stop compilation bug  Defect  CSSD100005755
        //ignoreList.insert("unstructed_cf7"); //different results View  Defect  CSSD100005766
       // ignoreList.insert("unstructed_cf6"); //never stop compilation   Defect  CSSD100005769
       // ignoreList.insert("infinite_loop7");//vectored version of kernel stop execution despite it include infinity loop  CSSD100005786
       // ignoreList.insert("infinite_loop6");//this kernel infinity loop but printf different outputs on the dysplay 
       // ignoreList.insert("infinite_loop4");// different results View  Defect  CSSD100005795
       // ignoreList.insert("atomic_func06");//different results bug open Defect CSSD100005848 
      //  ignoreList.insert("wia92_addsub"); //bug opened Defect:  CSSD100005651
        // ignoreList.insert("atom_func07");    //Defect  CSSD100005936

}

/*
 * runs all tests in the given directory and in all its sub directories.
 * updates the test results according to number of tests that were ran,
 * failed, succeeded, ignored.
 *
 * @param dirName
 * 		name of the directory which tests need to be executed
 * @param results
 * 		the results that should be updated according to the results of the test
 * 		ran in the given directory.
 */
void TestsRunner::runTestsInDir(const string& dirName, TestResults& results) {

#ifdef WINDOWS
	HANDLE handleFind;
	WIN32_FIND_DATA FindFileData;
    WIN32_FIND_DATAW u_FindFileData;
    int i,index;


    CHAR NPath[MAX_PATH]; 
    DWORD rez = GetCurrentDirectory(MAX_PATH,NPath); 
    string st_path=string(NPath);
    int found=st_path.rfind("build");
    st_path.erase(found);
    st_path.insert(found,dirName);
    while ((index=st_path.find('/',0))!=-1) {
        st_path.replace(index,1,"\\");
    }

    USES_CONVERSION;     
    string new_dirName="\\\\?\\";
    new_dirName.append(st_path.c_str());   
    //bool cantFindDir = (handleFind = FindFirstFile(st_path.c_str(),	&FindFileData)) == INVALID_HANDLE_VALUE;
   
    LPWSTR lpUnicodeStr = A2W(new_dirName.c_str() ) ;
    bool u_cantFindDir = (handleFind = FindFirstFileW(lpUnicodeStr,
			&u_FindFileData)) == INVALID_HANDLE_VALUE;

#else
	DIR *dir = opendir(dirName.c_str());
	bool cantFindDir = dir == NULL;
#endif

	if (u_cantFindDir) {
		throw TestFailedException("cannot open tests dir: " + string(st_path));
	}

#ifdef WINDOWS
    //dirName=st_path  
	//bool canFindTest = (handleFind = FindFirstFile(string(st_path + "\\*").c_str(), &FindFileData)) != INVALID_HANDLE_VALUE;
    new_dirName.append("\\*");   
    lpUnicodeStr = A2W(new_dirName.c_str() ) ;
    bool u_canFindTest = (handleFind = FindFirstFileW(lpUnicodeStr,
		&u_FindFileData)) != INVALID_HANDLE_VALUE;
#else
	struct dirent *test = readdir(dir);
	bool canFindTest = test;
#endif

	while (u_canFindTest) {

#ifdef WINDOWS
		string filename =W2A( u_FindFileData.cFileName);
		bool isDir = u_FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		// go to next file
		u_canFindTest = (FindNextFileW(handleFind,	&u_FindFileData));
#else
		string filename = test->d_name;
		bool isDir = test->d_type == DT_DIR;

		// go to next file
		test = readdir(dir);
		canFindTest = test;
#endif

		// these directories need to be ignored (include ., .., .svn)
		if (filename[0] == '.') {
			continue;
		}

		// run tests in sub directory recursively
		if (isDir) {
			runTestsInDir(dirName + "/" + filename, results);
			continue;
		}

		results.total++;

		string testName = filename;

		// check if test should be ignored
		if ((ignoreList.count(filename) > 0) || !isInTestsToRun(testName,
				dirName)) {
			results.ignored++;
			continue;
		}

		// run test, check for failure/success
		if (RunTest(testName, st_path)) {
			results.succeeded++;
		} else {
			results.failed++;
		}
	}
}

bool TestsRunner::isInTestsToRun(string testName, string path) {

	// no tests to run were set, assuming that need to run all tests
	if (testsToRun.empty()) {
		return true;
	}

	// run the test only if one of it's containing directories was specified,
	// or if the the test was specified specifically

	set<string>::iterator it;
	for (it = testsToRun.begin(); it != testsToRun.end(); it++) {
		string pathToRun = *it;

		if ((pathToRun == path) || (pathToRun + "/" == path.substr(0,
				pathToRun.size() + 1)) || (pathToRun == testName)) {
			return true;
		}
	}

	return false;
}

/*
 * runs all test files directly under TESTS_DIR, prints results, how many tests
 * were ran, how many succeeded and how many failed.
 */
int TestsRunner::RunAllTests() {

	TestResults results;

	// initialize test results
	results.total = 0;
	results.succeeded = 0;
	results.failed = 0;
	results.ignored = 0;

	// run all tests in tests base directory and all its sub directories
	runTestsInDir(TESTS_BASE_DIR, results);

	//print summary of the test run
	cout << "---------------------------------------------" << endl;
	cout << "Summary:" << endl << endl;

	cout << "total number tests: " << results.total << endl;
	cout << "number tests ignored: " << results.ignored << endl;
	cout << endl;
	cout << "number tests succeeded: " << results.succeeded << endl;
	cout << "number tests failed: " << results.failed << endl << endl;
	cout << "random generator seed: "
			<< RandomInputGenerator::getInstance().getSeed() << endl;
	cout << "---------------------------------------------" << endl;

	if (results.failed == 0) {
		cout << "=============================================" << endl;
		cout << "              All tests succeeded            " << endl;
		cout << "=============================================" << endl;
	}
    return results.failed;
}

/*
 * runs the given test. returns true in case the test succeeds, prints to cout
 * error message and return false otherwise.
 *
 * @param testName
 * 		name of the test to run
 * @param path
 * 		path of the test to run
 * @return
 * 		true in case test succeeded, false otherwise.
 */
bool TestsRunner::RunTest(const string& testName, const string& path) {

	int NUM_INPUTS = 1;
    time_t start,between,end;
    static int count=0;
	// vector of input arrays
	vector<float*> normalInputs;
	// vector of input arrays
	vector<float*> vectorizedInputs;

	try {
		float normalOutput[OUTPUT_SIZE];
		float vectorizedOutput[OUTPUT_SIZE];
		for (unsigned int i = 0; i < OUTPUT_SIZE; i++) {
			normalOutput[i] = 0;
			vectorizedOutput[i] = 0;
		}

		//		cout << endl;
		//		cout << "input: " << endl;

		for (int i = 0; i < NUM_INPUTS; i++) {
			float *normalInput = new float[INPUT_SIZE];
			float *vectorizedInput = new float[INPUT_SIZE];

			RandomInputGenerator::getInstance().getFloatGen().getArrayNums(
					normalInput, INPUT_SIZE);

			for (unsigned int j = 0; j < INPUT_SIZE; j++) {
				//				float val = RandomInputGenerator::getInstance().getFloatGen().getNum();
				//				normalInput[j] = val;
				//				vectorizedInput[j] = val;

				vectorizedInput[j] = normalInput[j];

				//				cout << normalInput[j] << ", ";
			}
			normalInputs.push_back(normalInput);
			vectorizedInputs.push_back(vectorizedInput);
		}

		//		cout << endl << endl;

		list<RegularParameter> arguments = getArguments(path);
        cout << "number of tests" << count <<"\n";     
        cout << "random generator seed: "
			<< RandomInputGenerator::getInstance().getSeed() << endl;             
		
        cout << testName << ", path: " << path <<" without vectorizer" << endl;
        start = time(NULL);        
        execute(testName, path, normalInputs, (float**) (&normalOutput),
				arguments, Execution::NORMAL);
        between = time(NULL);

        cout << testName << ", path: " << path <<"  with vectorizer" << endl;
        execute(testName, path, vectorizedInputs,
			(float**) (&vectorizedOutput), arguments, Execution::VECTORIZED);
        end = time(NULL);

        if( ((end-between)/4-2) >(between-start))

        cout << testName << "    warning" << path <<"  possible performance gap " << end-between <<" sec   " << between-start << " sec"<<endl;
        
    
        count = count +1;
		//cout << testName << ", path: " << path << endl;
		for (unsigned int i = 0; i < OUTPUT_SIZE; i++) {
			if (normalOutput[i] != vectorizedOutput[i]) {

				// TODO : treat NaNs (created by conversions from long to float) better than this

				if (IS_NAN(normalOutput[i]) && IS_NAN(vectorizedOutput[i])) {
					continue;
				}

				throw TestFailedException("bad output for kernel " + testName
						+ "\n\texpected: \t" + StringConverter::toString(

						normalOutput, OUTPUT_SIZE) + "\n\t but was: \t"
						+ StringConverter::toString(vectorizedOutput,
								OUTPUT_SIZE));
			}
		}

		cout << StringConverter::toString(normalOutput, OUTPUT_SIZE) << endl		<< endl;
		clearInputs(normalInputs, vectorizedInputs);
		return true;

	} catch (TestFailedException e) {
		handleException(testName, path, e, normalInputs, vectorizedInputs);
		return false;

	} catch (IllegalParameterException e) {
		handleException(testName, path, e, normalInputs, vectorizedInputs);
		return false;
	}
}

void TestsRunner::clearInputs(vector<float*> & normalInputs,
		vector<float*> & vectorizedInputs) {
	while (!normalInputs.empty()) {
		delete[] normalInputs.front();
		normalInputs.pop_back();
	}
	while (!vectorizedInputs.empty()) {
		delete[] vectorizedInputs.front();
		vectorizedInputs.pop_back();
	}
}

void TestsRunner::handleException(const string & testName, const string & path,
		exception& e, vector<float*> normalInputs,
		vector<float*> vectorizedInputs) {

	cout << "TEST FAILED" << endl;
	cout << "test: " << testName << ", path: " << path << endl;
	cout << "message: " << e.what() << endl << endl;

	clearInputs(normalInputs, vectorizedInputs);
}

/*
 * prepares all relevant parameter of the given test for the KernelExecutor and
 * executes the test.
 *
 * @param testName
 * 		the test to execute
 * @param path
 * 		path of the test to run
 * @param input
 * 		input parameter of the test
 * @param res
 * 		output parameter of the test
 * @param executionType
 * 		NORMAL denotes normal execution, VECTORIZED denotes execution using vectorizer.
 */
void TestsRunner::execute(const string& testName, const string& path,
		const vector<float*>& inputArrays, float** outputArray, const list<
				RegularParameter>& inputArgs,
		const Execution::Type executionType) {

	string kernelName = testName;
	string program_source = path + "\\" + kernelName;

	vector<ArrayParameter> params;

	// this allows several input arrays
	for (unsigned int i = 0; i < inputArrays.size(); i++) {

		float* input = inputArrays[i];

		ArrayParameter paramSrc(input, sizeof(float) * INPUT_SIZE,
				ArrayParameter::READ_ONLY);
		params.push_back(paramSrc);
	}

	ArrayParameter paramDest(outputArray, sizeof(float) * OUTPUT_SIZE,
			ArrayParameter::READ_WRITE);
	params.push_back(paramDest);

#ifdef WINDOWS
	//string includeFile = "C:/Users/ikleine1/Desktop/CVCC/OCL_VEC_february/tests/OCL_Vectorizer_Tests/tests/src/VectorizerTests/IncludeFiles/include.h";
    //string includeFile = "src/VectorizerTests/IncludeFiles/include.h";
    string includeFile = "../../../../tests/OCL_Vectorizer_Tests/tests/src/VectorizerTests/IncludeFiles/include.h";
#else
	string includeFile = "src/VectorizerTests/IncludeFiles/include.h";
#endif

	string executionRes =  KerenelExecutor::execKernel(kernelName,
			program_source, includeFile, NUM_INSTANCES, params, inputArgs,
			executionType);

	while (!params.empty()) {
		params.pop_back();
	}

	if (executionRes != "ok") {

		throw TestFailedException("kernel " + program_source
				+ " didn't execute properly in mode: " + Execution::toString(
				executionType) + ", reason: " + executionRes);
	}

}

list<RegularParameter> TestsRunner::getArguments(const string& path) {

	list<RegularParameter> argumentsList;

	size_t pos = path.find_first_of('\\');
	// the adding of "/" is done in order to take into account the last directory
	// in the path, because the path doesn't end with "/"
	string restOfPath = path + "\\";

	while (pos != string::npos) {

		string type = restOfPath.substr(0, pos);

		if (RegularParameter::isTypeLegal(type)) {
			argumentsList.push_back(RegularParameter(type));
		}

		restOfPath = restOfPath.substr(pos + 1);
		pos = restOfPath.find_first_of('\\');
	}

	return argumentsList;
}

