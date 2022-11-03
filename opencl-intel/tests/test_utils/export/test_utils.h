// INTEL CONFIDENTIAL
//
// Copyright 2012-2021 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "CL/cl.h"
#include "cl_types.h"
#include "cl_utils.h"
#include "llvm/Support/Regex.h"
#include <exception>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

bool CheckCondition(const char *name, bool condition);
bool SilentCheckCondition(const char *name, bool condition);
bool Check(const char *name, cl_int expected, cl_int result);
bool SilentCheck(const char *name, cl_int expected, cl_int result);
bool CheckStr(const char *name, const char *expected, char *result);
bool SilentCheckStr(const char *name, char *expected, char *result);
bool CheckInt(const char *name, cl_long expected, cl_long result);
bool SilentCheckInt(const char *name, cl_long expected, cl_long result);
bool CheckSize(const char *name, size_t expected, size_t result);
bool SilentCheckSize(const char *name, size_t expected, size_t result);
bool CheckBuildStatus(const char *name, cl_build_status expected,
                      cl_build_status result);
bool SilentCheckBuildStatus(const char *name, cl_build_status expected,
                            cl_build_status result);
bool BuildProgramSynch(cl_context context, cl_uint count, const char **strings,
                       const size_t *lengths, const char *options,
                       cl_program *program_ret);
void GetBuildLog(cl_device_id device, cl_program program, std::string &log);

/// Read binaries from \p srcProgram, create a new program from the binaries and
/// build the new program. The new program is saved to \p dstProgram.
void CreateAndBuildProgramFromProgramBinaries(cl_context context,
                                              cl_device_id device,
                                              const std::string &buildOptions,
                                              cl_program srcProgram,
                                              cl_program &dstProgram);

bool CheckHandle(const char *name, cl_platform_id expected,
                 cl_platform_id result);
bool CheckHandle(const char *name, cl_device_id expected, cl_device_id result);
bool CheckHandle(const char *name, cl_context expected, cl_context result);
bool CheckHandle(const char *name, cl_command_queue expected,
                 cl_command_queue result);
bool CheckHandle(const char *name, cl_mem expected, cl_mem result);
bool CheckHandle(const char *name, cl_program expected, cl_program result);
bool CheckHandle(const char *name, cl_kernel expected, cl_kernel result);
bool CheckHandle(const char *name, cl_event expected, cl_event result);
bool CheckHandle(const char *name, cl_sampler expected, cl_sampler result);
bool CheckHandleImpl(const char *name, void *expected, void *result, bool bRes);

// auxiliary functions for SilentCheckException

template <typename T> bool Compare(T x, T y) { return x == y; }

template <> bool Compare<const char *>(const char *x, const char *y);

template <typename T> void Print(std::ostream &os, const T &x) { os << x; }

template <> void Print<cl_int>(std::ostream &os, const cl_int &x);

/**
 * Check whether a result is as expected and print a message only in case of
 * failure. In this case an exception is also thrown.
 * @param T the type of the expected and result values
 * @param name the name of the check
 * @param expected the expected value
 * @param result the result value
 * @throw std::exception if expected does not equal result
 */
template <typename T>
void __CheckException__(const char *name, const T &expected, const T &result) {
  if (!Compare(expected, result)) {
    std::cout << "FAIL: " << name << std::endl;
    std::cout << "\t\texpected = ";
    Print(std::cout, expected);
    std::cout << ", result = ";
    Print(std::cout, result);
    std::cout << std::endl;
    throw std::exception();
  }
}

#define CheckException(name, expected, result)                                 \
  {                                                                            \
    char buf[1024];                                                            \
    SPRINTF_S(buf, 1024, "%s (%s:%d)", name, __FILE__, __LINE__);              \
    __CheckException__(buf, expected, result);                                 \
  }

#define CHECK_COND(name, cond)                                                 \
  {                                                                            \
    char buf[1024];                                                            \
    SPRINTF_S(buf, 1024, "%s (%s:%d)", name, __FILE__, __LINE__);              \
    if (!SilentCheckCondition(name, cond)) {                                   \
      throw std::exception();                                                  \
    }                                                                          \
  }

// A simple tokenizer - extracts a vector of tokens from a
// string, delimited by any character in delims.
//
std::vector<std::string> tokenize(const std::string &str,
                                  const std::string &delims);

// Start capturing stdout
//
bool CaptureStdout();

// Start capturing stderr
//
bool CaptureStderr();

// Stop capturing stdout and return the captured string
//
std::string GetCapturedStdout();

// Stop capturing stderr and return the captured string
//
std::string GetCapturedStderr();

// Split the expected and the actual outputs to lines, sort the lines and
// compare them.
bool compare_kernel_output(const string &expected, const string &actual);

cl_ulong trySetStackSize(cl_ulong size);

/// Return the maximum number of external threads that are allowed in
/// a test in which each thread launches a separate kernel.
unsigned getMaxNumExternalThreads();

/// Report error if the file doesn't contain all of the pattern strings.
void checkFileContains(const std::string &Filename,
                       const std::vector<std::string> &Patterns);

/// Return true if the file contains all of the pattern strings.
bool fileContains(const std::string &Filename,
                  const std::vector<std::string> &Patterns);

/// Find all files with name matching the regex in a directory.
std::vector<std::string> findFilesInDir(const std::string &Dir,
                                        const llvm::Regex &R);
