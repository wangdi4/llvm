// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
