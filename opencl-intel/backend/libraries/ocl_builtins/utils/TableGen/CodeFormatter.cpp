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

#include "CodeFormatter.h"
#include <algorithm>

namespace llvm {

CodeFormatter::~CodeFormatter() {}

void CodeFormatter::indent() { ++m_indentLevel; }

void CodeFormatter::unindent() {
  m_indentLevel = std::max(0, m_indentLevel - 1);
}

CodeFormatter::CodeFormatter(llvm::raw_ostream &s, int indent)
    : INDENT(indent), m_stream(s), m_shouldIndent(true) {
  m_indentLevel = 0;
}

CodeFormatter &CodeFormatter::endl() {
  m_shouldIndent = true;
  m_stream << "\n";
  return *this;
}

void CodeFormatter::writeIndentation() {
  int i = INDENT * m_indentLevel;
  assert(i < 20 && "indentation went wild");
  while (m_shouldIndent && i > 0) {
    i--;
    m_stream << ' ';
  }
  m_shouldIndent = false;
}

} // namespace llvm
