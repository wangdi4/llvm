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

#ifndef __SOURCE_FILE_H__
#define __SOURCE_FILE_H__

#include <string>
namespace Intel {
namespace OpenCL {
namespace Frontend {

struct BinaryBuffer {
  const void *binary;
  size_t size;
  BinaryBuffer();
  BinaryBuffer(const void *, size_t);
};

//
// Represents an OpenCL source file to be written to the configuration file
class SourceFile {
public:
  ////////////////////////////////////////////
  // Parameters:
  //   name: the name of the source file
  //   contents: the contents of the source file
  //   compilationFlags: the compiler flags by which the source file should be
  //   compiled.
  ////////////////////////////////////////////
  SourceFile(const std::string &name, const std::string &contents,
             const std::string &compilationFlags);
  SourceFile();
  // getters
  std::string getName() const;
  std::string getContents() const;
  std::string getCompilationFlags() const;
  BinaryBuffer getBinaryBuffer() const;
  // setters
  void setName(const std::string &name);
  void setContents(const std::string &contents);
  void setCompilationFlags(const std::string &flags);
  void setBinaryBuffer(const BinaryBuffer &buffer);
  bool operator==(const SourceFile &) const;

private:
  // the name of the cl source file
  std::string m_name;
  // contents of the source file
  std::string m_contents;
  // compilation by which the source file should compile with
  std::string m_compilationFlags;
  // holds the binary resulted from the compilation process of the source file
  BinaryBuffer m_binaryBuffer;
}; // End SourceFile

} // namespace Frontend
} // namespace OpenCL
} // namespace Intel
#endif //__SOURCE_FILE_H__
