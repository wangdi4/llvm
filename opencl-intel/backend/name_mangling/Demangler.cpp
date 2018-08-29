// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "DemangleParser.h"
#include "FunctionDescriptor.h"

#include <stdlib.h>

const char *PREFIX = "_Z";

size_t PREFIX_LEN = 2;
// parsing methods for the raw mangled name
static bool peelPrefix(llvm::StringRef &s) {
  if (PREFIX != s.substr(0, PREFIX_LEN))
    return false;
  s = s.substr(PREFIX_LEN, s.size() - PREFIX_LEN);
  return true;
}

// some platforms dont have isdigit defined
int ISDIGIT(int c) { return (c >= '0' && c <= '9'); }

static llvm::StringRef peelNameLen(llvm::StringRef &s) {
  int len = 0;
  llvm::StringRef::const_iterator it = s.begin();
  while (ISDIGIT(*it++))
    ++len;
  llvm::StringRef ret = s.substr(0, len);
  s = s.substr(len, s.size() - len);
  return ret;
}

// Implementation of an API function.
// Indicating whether the given string is a mangled function name
bool isMangledName(const char *rawString) {
  const std::string strName(rawString);
  if (strName.substr(0, PREFIX_LEN) != PREFIX)
    return false;
  return ISDIGIT(strName[PREFIX_LEN]);
}

// Implementation of an API function.
// converts the given mangled name string to a FunctionDescriptor object.
reflection::FunctionDescriptor demangle(const char *rawstring,
                                        bool isSpir12Name) {
  if (!rawstring || reflection::FunctionDescriptor::nullString() == rawstring)
    return reflection::FunctionDescriptor::null();

  llvm::StringRef mangledName(rawstring);
  // making sure it starts with _Z
  if (false == peelPrefix(mangledName)) {
    return reflection::FunctionDescriptor::null();
  }
  llvm::StringRef nameLen = peelNameLen(mangledName);
  // cutting the prefix
  int len = atoi(nameLen.data());
  llvm::StringRef functionName = mangledName.substr(0, len);
  llvm::StringRef parameters =
      mangledName.substr(len, mangledName.size() - len);

  reflection::FunctionDescriptor ret;

  reflection::DemangleParser parser(ret.parameters, isSpir12Name);

  if (!parser.demangle(parameters.data())) {
    return reflection::FunctionDescriptor::null();
  }

  ret.name = functionName.str();
  ret.width = reflection::width::NONE;

  return ret;
}

class DemanglerException : public std::exception {
public:
  const char *what() const throw() { return "invalid prefix"; }
};

// Implementation of an API function.
llvm::StringRef stripName(const char *rawstring) {
  llvm::StringRef mangledName(rawstring);
  // making sure it starts with _Z
  if (false == peelPrefix(mangledName))
    throw DemanglerException();
  llvm::StringRef nameLen = peelNameLen(mangledName);
  // cutting the prefix
  int len = atoi(nameLen.data());
  return llvm::StringRef(rawstring + PREFIX_LEN + nameLen.size(), len);
}
