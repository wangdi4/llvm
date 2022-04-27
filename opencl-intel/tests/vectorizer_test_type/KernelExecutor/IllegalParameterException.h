/*
 * ExceptionWithMsg.h
 *
 *  Created on: Apr 30, 2009
 *      Author: opencl
 */

#ifndef ILLEGALPARAMETEREXCEPTION_H_
#define ILLEGALPARAMETEREXCEPTION_H_


#include <string>
#include <exception>

using std::string;
using std::exception;

class IllegalParameterException : public exception {
public:

	IllegalParameterException(string msg) {
		this->msg = msg;
	}

	~IllegalParameterException() throw() {}

	virtual const char* what() const throw() { return msg.c_str(); }

private:
	string msg;
};

#endif /* ILLEGALPARAMETEREXCEPTION_H_ */
