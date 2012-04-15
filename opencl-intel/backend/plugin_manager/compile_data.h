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

File Name:  CompileData.h

\*****************************************************************************/
#ifndef __COMPILEDATA_H__
#define __COMPILEDATA_H__

#include <vector>
#include "source_file.h"

namespace Intel { namespace OpenCL { namespace Frontend {

class CompileData{
public:
  /////////////////////////////////////////////////////
  //Parameters:
  //  headers: a vector of headers included by the source file being compiled
  //  sourceFile: the source file being compiled by the compilation action,
  //  which caused the instance of this class to be created.
  /////////////////////////////////////////////////////
  CompileData(const std::vector<SourceFile>& headers, const SourceFile& sourceFile);
  CompileData(const CompileData&);
  CompileData();
  //
  //setters
  //
  // sets the compiled source file
  void sourceFile(const SourceFile&);
  //adds the given source file to the headers to be included by the source file
  void addIncludeFile(const SourceFile&);
  //
  //getters
  const SourceFile& sourceFile() const;
  std::vector<SourceFile>::const_iterator beginHeaders()const;
  std::vector<SourceFile>::const_iterator endHeaders()const;
private:
  //Source file compiled by the compilation action
  SourceFile m_sourceFile;
  //a list of headers included by the above source file
  std::vector<SourceFile> m_headers;

};

}}}


#endif
