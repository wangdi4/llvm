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

File Name:  CompileData.cpp

\*****************************************************************************/
#include "compile_data.h"
#include <algorithm>

namespace Intel { namespace OpenCL { namespace Frontend {
  CompileData::CompileData(){}

  CompileData::CompileData(const std::vector<SourceFile>& headers, 
    const SourceFile& sourceFile) : m_sourceFile(sourceFile){
    std::copy(headers.begin(), headers.end(), m_headers.begin());
  }
  
  CompileData::CompileData(const CompileData& that){
    this->m_sourceFile = that.sourceFile();
    std::copy(that.m_headers.begin(), that.m_headers.end(), m_headers.begin());
  }
  
  const SourceFile& CompileData::sourceFile() const{
    return m_sourceFile;
  }
  
  void CompileData::addIncludeFile(const SourceFile& header){
    m_headers.push_back(header);
  }

  void CompileData::sourceFile(const SourceFile& sourceFile){
    m_sourceFile = sourceFile;
  }

  std::vector<SourceFile>::const_iterator CompileData::beginHeaders() const{
    return m_headers.begin();
  }
  
  std::vector<SourceFile>::const_iterator CompileData::endHeaders() const{
    return m_headers.end();
  }

}}}
