// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "compile_data.h"
#include <algorithm>

namespace Intel {
namespace OpenCL {
namespace Frontend {
CompileData::CompileData() {}

CompileData::CompileData(const std::vector<SourceFile> &headers,
                         const SourceFile &sourceFile)
    : m_sourceFile(sourceFile) {
  std::copy(headers.begin(), headers.end(), m_headers.begin());
}

CompileData::CompileData(const CompileData &that) {
  this->m_sourceFile = that.sourceFile();
  std::copy(that.m_headers.begin(), that.m_headers.end(), m_headers.begin());
}

CompileData &CompileData::operator=(const CompileData &that) {
  this->m_sourceFile = that.sourceFile();
  std::copy(that.m_headers.begin(), that.m_headers.end(),
            this->m_headers.begin());
  return *this;
}

const SourceFile &CompileData::sourceFile() const { return m_sourceFile; }

void CompileData::addIncludeFile(const SourceFile &header) {
  m_headers.push_back(header);
}

void CompileData::sourceFile(const SourceFile &sourceFile) {
  m_sourceFile = sourceFile;
}

std::vector<SourceFile>::const_iterator CompileData::beginHeaders() const {
  return m_headers.begin();
}

std::vector<SourceFile>::const_iterator CompileData::endHeaders() const {
  return m_headers.end();
}

} // namespace Frontend
} // namespace OpenCL
} // namespace Intel
