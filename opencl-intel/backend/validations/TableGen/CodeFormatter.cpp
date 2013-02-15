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

File Name: CodeFormatter.cpp

\****************************************************************************/

#include "CodeFormatter.h"
#include <algorithm>

namespace llvm{

CodeFormatter::~CodeFormatter(){
}

void CodeFormatter::indent(){ ++m_indentLevel; }

void CodeFormatter::unindent(){ m_indentLevel = std::max(0, m_indentLevel-1); }

CodeFormatter::CodeFormatter (llvm::raw_ostream& s, int indent)
: INDENT(indent), m_stream(s), m_shouldIndent(true){
  m_indentLevel = 0;
}

CodeFormatter& CodeFormatter::endl(){
  m_shouldIndent = true;
  m_stream << "\n";
  return *this;
}

void CodeFormatter::writeIndentation(){
  int i = INDENT * m_indentLevel;
  assert(i < 20 && "indentation went wild");
  while(m_shouldIndent && i>0){
    i--;
    m_stream << ' ';
  }
  m_shouldIndent = false;
}

}//end reflection
