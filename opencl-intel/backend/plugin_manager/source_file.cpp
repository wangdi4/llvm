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

#include "source_file.h"
#include <stdio.h>

namespace Intel {
namespace OpenCL {
namespace Frontend {
//
// BinaryBuffer
//
BinaryBuffer::BinaryBuffer() : binary(nullptr), size(-1) {}

BinaryBuffer::BinaryBuffer(const void *_binary, size_t _size)
    : binary(_binary), size(_size) {}

//
// SourceFile
//
SourceFile::SourceFile(const std::string &name, const std::string &contents,
                       const std::string &compilationFlags)
    : m_name(name), m_contents(contents), m_compilationFlags(compilationFlags) {
}

SourceFile::SourceFile() {}

std::string SourceFile::getName() const { return m_name; }

void SourceFile::setName(const std::string &name) { m_name = name; }

std::string SourceFile::getContents() const { return m_contents; }

void SourceFile::setContents(const std::string &contents) {
  m_contents = contents;
}

std::string SourceFile::getCompilationFlags() const {
  return m_compilationFlags;
}

void SourceFile::setCompilationFlags(const std::string &flags) {
  m_compilationFlags = flags;
}

BinaryBuffer SourceFile::getBinaryBuffer() const { return m_binaryBuffer; }

void SourceFile::setBinaryBuffer(const BinaryBuffer &binaryBuffer) {
  m_binaryBuffer = binaryBuffer;
}

bool SourceFile::operator==(const SourceFile &that) const {
  if (this == &that)
    return true;
  return m_contents == that.m_contents;
}

} // namespace Frontend
} // namespace OpenCL
} // namespace Intel
