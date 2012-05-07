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

File Name: source_file.h

\*****************************************************************************/
#ifndef __SOURCE_FILE_H__
#define __SOURCE_FILE_H__

#include <string>
namespace Intel { namespace OpenCL { namespace Frontend {

struct BinaryBuffer{
  const void* binary;
  size_t size;
  BinaryBuffer();
  BinaryBuffer(const void*, size_t);
};

//
//Represents an OpenCL source file to be written to the configuration file
class SourceFile{
public:
  ////////////////////////////////////////////
  //Parameters:
  //  name: the name of the source file
  //  contents: the contents of the source file
  //  compilationFlags: the compiler flags by which the source file should be
  //  compiled.
  ////////////////////////////////////////////
  SourceFile(const std::string& name, const std::string& contents,
    const std::string& compilationFlags);
  SourceFile();
  //getters
  std::string getName() const;
  std::string getContents() const;
  std::string getCompilationFlags() const;
  BinaryBuffer getBinaryBuffer() const;
  //setters
  void setName(const std::string& name);
  void setContents(const std::string& contents);
  void setCompilationFlags(const std::string& flags);
  void setBinaryBuffer(const BinaryBuffer& buffer);
  bool operator == (const SourceFile&) const;
private:
  //the name of the cl source file
  std::string m_name;
  //contents of the source file
  std::string m_contents;
  //compilation by which the source file should compile with
  std::string m_compilationFlags;
  //holds the binary resulted from the compilation process of the source file
  BinaryBuffer m_binaryBuffer;
};//End SourceFile

}}}
#endif //__SOURCE_FILE_H__
