/****************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: CodeFormatter.h

\****************************************************************************/

#ifndef __CODE_FORMATTER_H__
#define __CODE_FORMATTER_H__

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringRef.h"
#include <list>

namespace llvm{

///////////////////////
//Purpose: design to help code generation in a formated way.
///////////////////////
struct CodeFormatter{
  ~CodeFormatter();

  void indent();

  void unindent();

  CodeFormatter (llvm::raw_ostream& s, int indent=2);

  template <typename T>
  CodeFormatter& emmit (const T& obj);

  CodeFormatter& endl();

private:
  void writeIndentation();

  const int INDENT;
  int m_indentLevel;
  llvm::raw_ostream& m_stream;
  bool m_shouldIndent;
};

template <typename T>
CodeFormatter& CodeFormatter::emmit (const T& obj){
  writeIndentation();
  m_stream << obj;
  return *this;
}

template <typename T>
CodeFormatter& operator << (CodeFormatter& cf, const T& obj){
  return cf.emmit(obj);
}

}

#endif//__CODE_FORMATTER_H__
