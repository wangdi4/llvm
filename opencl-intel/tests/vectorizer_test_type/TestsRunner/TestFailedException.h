/*
 * ExceptionWithMsg.h
 *
 *  Created on: Apr 30, 2009
 *      Author: opencl
 */

#ifndef EXCEPTIONWITHMSG_H_
#define EXCEPTIONWITHMSG_H_

#include <exception>
#include <string>

using std::exception;
using std::string;

class TestFailedException : public exception {
public:
  TestFailedException(string msg) { this->msg = msg; }

  ~TestFailedException() throw() {}

  virtual const char *what() const throw() { return msg.c_str(); }

private:
  string msg;
};

#endif /* EXCEPTIONWITHMSG_H_ */
