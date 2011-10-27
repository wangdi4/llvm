#include <string>
#include <set>

#include "KernelExecutor/KerenelExecutor.h"
#include "TestsRunner/TestsRunner.h"

using std::string;
using std::set;

int main(int argc, char * const argv[]) {

	KerenelExecutor::setPauseExecution(false);
    int rezult=0;
    string updated_code_str;
    string::iterator it; 
    char buffer[2000];
    int counter=0;

	// check if need to pause execution before and after running the kernel

	set<string> testsToRun;
    set<string> testsNotToRun;

	for (int i = 1; i < argc; i++) {

        
		string param = argv[i];

		if (argv[1] == "-pause") {
			KerenelExecutor::setPauseExecution(true);
		} 
        else  if (argv[1][0] =='i')
        {
			testsNotToRun.insert(param);
		}
        else if (argv[1][0] =='r')
        {
			testsToRun.insert(param);
		}

        //read ignore list parameters from file
        else if (argv[1][0] =='f')
        {
            string code = KerenelExecutor::getFileContent("../../../../tests/OCL_Vectorizer_Tests/tests/src/VectorizerTests/teststoignore.txt");
            for ( it=code.begin() ; it < code.end(); it++ )
            {
                buffer[counter]=*it;
                counter++;
                if (*it==' ')
                {
                    buffer[counter-1]='\0'; 
                    counter=0;
                    param=buffer;
                    testsNotToRun.insert(param);
                }

            }
            buffer[counter]='\0'; 
            param=buffer;
            testsNotToRun.insert(param);        
        }
	}

	TestsRunner::setIgnoreTestList(testsNotToRun);
    TestsRunner::setTestList(testsToRun);
	
	TestsRunner noInputArgs;

	rezult=noInputArgs.RunAllTests();

    if (rezult>0)
    {
        return -1;
    }

#ifdef WINDOWS
	//getchar();
#else
#endif

	return 0;
}

