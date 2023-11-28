/*
 * KerenelExecutor.h
 *
 *  Created on: May 7, 2009
 *      Author: opencl
 */

#ifndef KERENELEXECUTOR_H_
#define KERENELEXECUTOR_H_

#include "ArrayParameter.h"

#ifdef WINDOWS
#include "CL/cl.h"
#else
#include "CL/opencl.h"
#endif
#include "Execution.h"
#include "RegularParameter.h"
#include <list>
#include <string>
#include <vector>

using std::list;
using std::string;
using std::vector;

class KerenelExecutor {
public:
  static void setPauseExecution(bool newVal);

  static string execKernel(const string &kernelName,
                           const string &program_source,
                           const string &includeFile, size_t numInstances,
                           vector<ArrayParameter> &params,
                           list<RegularParameter> inputArgs,
                           Execution::Type executionType);

private:
  static bool pauseExecution;
  static void deleteMemobjs(cl_mem *memobjs, int n);
  static string getFileContent(const string &fileName);
  static cl_program createProgramFromFile(cl_context context,
                                          const string &fileName,
                                          const string &includeFile);
};

#endif /* KERENELEXECUTOR_H_ */
