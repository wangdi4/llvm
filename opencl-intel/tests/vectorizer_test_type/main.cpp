#include <string>
#include <set>

#include "KernelExecutor/KerenelExecutor.h"
#include "TestsRunner/TestsRunner.h"

using std::string;
using std::set;

int main(int argc, char * const argv[]) {

	KerenelExecutor::setPauseExecution(false);

	// check if need to pause execution before and after running the kernel

	set<string> testsToRun;
	for (int i = 1; i < argc; i++) {

		string param = argv[i];

		if (param == "-pause") {
			KerenelExecutor::setPauseExecution(true);
		} else {
			testsToRun.insert(param);
		}
	}

	TestsRunner::setTestList(testsToRun);

	TestsRunner noInputArgs;

	noInputArgs.RunAllTests();

#ifdef WINDOWS
	getchar();
#else
#endif

	return 0;
}

