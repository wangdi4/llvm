#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include <vector>

// Read the contents of a file into a string. If there's a problem opening the
// file return an empty string.
//
std::string read_file_contents(std::string filename);


// Split a string to tokens by the given delimiters
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

#endif // TEST_UTILS_H


