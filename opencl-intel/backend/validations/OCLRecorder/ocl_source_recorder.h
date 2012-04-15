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

File Name:  ocl_source_recorder.cpp

\*****************************************************************************/

#ifndef __OCL_SOURCE_RECORDER_H__
#define __OCL_SOURCE_RECORDER_H__

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include "plugin_interface.h"
#include "source_file.h"
#include "link_data.h"
#include "compile_data.h"
#include "md5.h"

namespace Validation{

//
//Functional class for MD5 code comparision, to be used by code map for partial
//ordering.
//
struct CodeLess{
  //returns true id x < y, false otherwise
  bool operator () (const MD5Code& x, const MD5Code& y) const;
};

typedef std::map<MD5Code, Intel::OpenCL::Frontend::SourceFile, CodeLess> SourceFileMap;
typedef SourceFileMap::const_iterator FileIter;

class OclSourceRecorder: public Intel::OpenCL::Frontend::ICLFrontendPlugin{
public:
  //Type definitions
  //
  typedef std::vector<Intel::OpenCL::Frontend::SourceFile> SourceFilesVector;

  ~OclSourceRecorder();
  //
  //invoked when a program is being linked
  void OnLink(const Intel::OpenCL::Frontend::LinkData* linkData);
  //
  //invoked when a program is being compiled
  void OnCompile(const Intel::OpenCL::Frontend::CompileData* compileData);
  //
  //begin iterator for the files which need to be generated for a configuration
  //file
  FileIter begin(const MD5Code&) const;
  //
  //end iterator for the files which need to be generated for a configuration
  //file
  FileIter end() const;
private:
  //map of compiled source files. Maps the md5 of the binary,
  //to its corresponding source file.
  mutable SourceFileMap m_sourceMap;
  //lock for the source map
  mutable llvm::sys::Mutex m_sourcemapLock;
  //vector of headers included by the above sourece files
  SourceFilesVector m_headers;
  //lock for the headers vectors
  llvm::sys::Mutex m_headersLock;
};//end class
}

#endif //__OCL_SOURCE_RECORDER_H__
