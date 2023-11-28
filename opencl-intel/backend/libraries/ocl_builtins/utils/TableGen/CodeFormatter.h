// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __CODE_FORMATTER_H__
#define __CODE_FORMATTER_H__

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include <list>

namespace llvm {

///////////////////////
// Purpose: design to help code generation in a formated way.
///////////////////////
struct CodeFormatter {
  ~CodeFormatter();

  void indent();

  void unindent();

  CodeFormatter(llvm::raw_ostream &s, int indent = 2);

  template <typename T> CodeFormatter &emmit(const T &obj);

  CodeFormatter &endl();

private:
  void writeIndentation();

  const int INDENT;
  int m_indentLevel;
  llvm::raw_ostream &m_stream;
  bool m_shouldIndent;
};

template <typename T> CodeFormatter &CodeFormatter::emmit(const T &obj) {
  writeIndentation();
  m_stream << obj;
  return *this;
}

template <typename T>
CodeFormatter &operator<<(CodeFormatter &cf, const T &obj) {
  return cf.emmit(obj);
}

} // namespace llvm

#endif //__CODE_FORMATTER_H__
