#pragma once

#include <string>
#include <vector>
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
