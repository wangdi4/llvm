#pragma once

#include <string>
#include <vector>
#include <exception>
#include <iostream>
#include "CL/cl.h"
#include "cl_types.h"
#include "cl_utils.h"

bool CheckCondition(wchar_t * name, bool condition);
bool SilentCheckCondition(wchar_t * name, bool condition);
bool Check(wchar_t * name, cl_int expected, cl_int result);
bool SilentCheck(wchar_t * name, cl_int expected, cl_int result);
bool CheckStr(wchar_t * name, char * expected, char * result);
bool SilentCheckStr(wchar_t * name, char * expected, char * result);
bool CheckInt(wchar_t * name, cl_long expected, cl_long result);
bool SilentCheckInt(wchar_t * name, cl_long expected, cl_long result);
bool CheckSize(wchar_t * name, size_t expected, size_t result);
bool SilentCheckSize(wchar_t * name, size_t expected, size_t result);
bool CheckBuildStatus(wchar_t * name, cl_build_status expected, cl_build_status result);
bool SilentCheckBuildStatus(wchar_t * name, cl_build_status expected, cl_build_status result);
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
void Print(std::wostream& os, const T& x)
{
    os << x;
}

template<>
void Print<cl_int>(std::wostream& os, const cl_int& x);

/**
 * Check whether a result is as expected and print a message only in case of failure. In this case an exception is also thrown.
 * @param T the type of the expected and result values
 * @param name the name of the check
 * @param expected the expected value
 * @param result the result value
 * @throw std::exception if expected does not equal result
 */
template<typename T>
void CheckException(const wchar_t* name, const T& expected, const T& result)
{
    if (!Compare(expected, result))
    {
        std::wcout << L"FAIL: " << name << std::endl;
        std::wcout << L"\t\texpected = ";
        Print(std::wcout, expected);
        std::wcout << L", result = ";
        Print(std::wcout, result);
        std::wcout << std::endl;
        throw std::exception();
    }
}

// A simple tokenizer - extracts a vector of tokens from a 
// string, delimited by any character in delims.
//
std::vector<std::string> tokenize(const std::string& str, const std::string& delims);

// Start capturing stdout
//
void CaptureStdout();

// Start capturing stderr
//
void CaptureStderr();

// Stop capturing stdout and return the captured string
//
std::string GetCapturedStdout();

// Stop capturing stderr and return the captured string
//
std::string GetCapturedStderr();
