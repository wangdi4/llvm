#pragma once

#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include <stdlib.h>
#include "CL/cl.h"
#include "cl_types.h"
#include "cl_utils.h"

#if defined(_WIN32)
#define SETENV(NAME,VALUE)      (_putenv_s(NAME,VALUE) == 0)
#define UNSETENV(NAME)          (_putenv_s(NAME,"") == 0)
#else
#define SETENV(NAME,VALUE)      (setenv(NAME,VALUE,1) == 0)
#define UNSETENV(NAME)          (unsetenv(NAME) == 0)
#endif

bool CheckCondition(const char * name, bool condition);
bool SilentCheckCondition(const char * name, bool condition);
bool Check(const char * name, cl_int expected, cl_int result);
bool SilentCheck(const char * name, cl_int expected, cl_int result);
bool CheckStr(const char * name, const char * expected, char * result);
bool SilentCheckStr(const char * name, char * expected, char * result);
bool CheckInt(const char * name, cl_long expected, cl_long result);
bool SilentCheckInt(const char * name, cl_long expected, cl_long result);
bool CheckSize(const char * name, size_t expected, size_t result);
bool SilentCheckSize(const char * name, size_t expected, size_t result);
bool CheckBuildStatus(const char * name, cl_build_status expected, cl_build_status result);
bool SilentCheckBuildStatus(const char * name, cl_build_status expected, cl_build_status result);
bool BuildProgramSynch(cl_context context, cl_uint count, const char ** strings, const size_t * lengths, const char * options, cl_program * program_ret);

// auxiliary functions for SilentCheckException

template<typename T>
bool Compare(T x, T y)
{
    return x == y;
}

template<>
bool Compare<const char*>(const char* x, const char* y);

template<typename T>
void Print(std::ostream& os, const T& x)
{
    os << x;
}

template<>
void Print<cl_int>(std::ostream& os, const cl_int& x);

/**
 * Check whether a result is as expected and print a message only in case of failure. In this case an exception is also thrown.
 * @param T the type of the expected and result values
 * @param name the name of the check
 * @param expected the expected value
 * @param result the result value
 * @throw std::exception if expected does not equal result
 */
template<typename T>
void __CheckException__(const char* name, const T& expected, const T& result)
{
    if (!Compare(expected, result))
    {
        std::cout << "FAIL: " << name << std::endl;
        std::cout << "\t\texpected = ";
        Print(std::cout, expected);
        std::cout << ", result = ";
        Print(std::cout, result);
        std::cout << std::endl;
        throw std::exception();
    }
}

#define CheckException(name, expected, result) { \
	char buf[1024]; \
	SPRINTF_S(buf, 1024, "%s (%s:%d)", name, __FILE__, __LINE__); \
	__CheckException__(buf, expected, result); \
}

#define CHECK_COND(name, cond) { \
	char buf[1024]; \
	SPRINTF_S(buf, 1024, "%s (%s:%d)", name, __FILE__, __LINE__); \
	if (!SilentCheckCondition(name, cond)) \
	{ \
		throw std::exception(); \
	} \
}

// A simple tokenizer - extracts a vector of tokens from a 
// string, delimited by any character in delims.
//
std::vector<std::string> tokenize(const std::string& str, const std::string& delims);

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
