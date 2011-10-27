/*
 * KerenelExecutor.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef KERENELEXECUTOR_H_
#define KERENELEXECUTOR_H_

#include <string>
#include <vector>
#include <list>

#ifdef WINDOWS
#include <CL/cl.h>
#else
#include <opencl.h>
#endif


#include  "Execution.h"
#include "ArrayParameter.h"
#include "RegularParameter.h"

using std::string;
using std::vector;
using std::list;

class KerenelExecutor {
public:

	static void setPauseExecution(bool newVal);
    static string getFileContent(const string& fileName);
   

	static string
			execKernel(const string& kernelName, const string& program_source,
					const string& includeFile, size_t numInstances, vector<
							ArrayParameter>& params,
					list<RegularParameter> inputArgs,
					Execution::Type executionType);

private:

	static bool pauseExecution;
	static void deleteMemobjs(cl_mem *memobjs, int n);
    static string update_code_vec_type(const string& code ,Execution::Type executionType);
	static cl_program createProgramFromFile(cl_context context,
			const string& fileName, const string& includeFile, Execution::Type executionType);
};

#endif /* KERENELEXECUTOR_H_ */

#define STRING_DISABLE_VECTORIZER "__attribute__((vec_type_hint(float4)))\n"
#define KERNEL_STRING "__kernel"
#define KERNEL_STRING_SIZE (9)

