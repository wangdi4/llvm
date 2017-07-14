/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: source_file.cpp

\*****************************************************************************/
#include "source_file.h"

#include <stdio.h>

namespace Intel { namespace OpenCL { namespace Frontend {
  //
  //BinaryBuffer
  //
  BinaryBuffer::BinaryBuffer(): binary(nullptr), size(-1){
  }

  BinaryBuffer::BinaryBuffer(const void* _binary, size_t _size):
  binary(_binary), size(_size){
  }

  //
  //SourceFile
  //
  SourceFile::SourceFile(const std::string& name, const std::string& contents,
    const std::string& compilationFlags):
    m_name(name),
    m_contents(contents),
    m_compilationFlags(compilationFlags){
  }

  SourceFile::SourceFile(){
  }

  std::string SourceFile::getName() const{
    return m_name;
  }

  void SourceFile::setName(const std::string& name){
    m_name = name;
  }

  std::string SourceFile::getContents() const{
    return m_contents;
  }

  void SourceFile::setContents(const std::string& contents){
    m_contents = contents;
  }

  std::string SourceFile::getCompilationFlags() const{
    return m_compilationFlags;
  }

  void SourceFile::setCompilationFlags(const std::string& flags){
    m_compilationFlags = flags;
  }

  BinaryBuffer SourceFile::getBinaryBuffer()const{
    return m_binaryBuffer;
  }

  void SourceFile::setBinaryBuffer(const BinaryBuffer& binaryBuffer){
    m_binaryBuffer = binaryBuffer;
  }

  bool SourceFile::operator ==(const SourceFile& that) const{
    if (this == &that)
      return true;
    return m_contents == that.m_contents;
  }

}}}//end namespace
