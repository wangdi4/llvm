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

#ifndef __OCL_SOURCE_RECORDER_H__
#define __OCL_SOURCE_RECORDER_H__

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include "llvm/Support/Mutex.h"
#include "plugin_interface.h"
#include "source_file.h"
#include "link_data.h"
#include "compile_data.h"
#include "md5.h"

namespace Validation{

//
//Functional class for MD5 code comparison, to be used by code map for partial
//ordering.
//
struct CodeLess{
  //returns true id x < y, false otherwise
  bool operator () (const MD5Code& x, const MD5Code& y) const;
};

//
//Type definitions
//
typedef std::vector<Intel::OpenCL::Frontend::SourceFile> SourceFilesVector;

typedef std::map<MD5Code, SourceFilesVector, CodeLess> SourceFileMap;

class OclSourceRecorder;
//
//SourceFile iterator
//
class FileIter {
  friend class OclSourceRecorder;
public:
  FileIter();
  FileIter& operator++();
  FileIter operator++(int);
  bool operator==(const FileIter&)const;
  bool operator!=(const FileIter&)const;
  Intel::OpenCL::Frontend::SourceFile operator * ()const;
  static FileIter end();
  struct FileIterException: public std::exception{
  public:
    const char* what() const throw();
  };
private:

  FileIter(SourceFilesVector::const_iterator b, SourceFilesVector::const_iterator e);
  SourceFilesVector::const_iterator m_iter;
  SourceFilesVector::const_iterator m_iterEnd;
  bool m_isInitialized;
  bool m_isExhausted;
};

//@Name: OclSourceRecorder
//@Description: implementing the interface <code>ICLFrontendPlugin</code>,
//this class supplies call back methods for Link and Compile events. Those
//callback methods builds an internal dependency graph between compilation
//artifacts. The <code>begin</code> method can than be used to query all the
//dependent artifacts of a given module (or more precisely, the MD5 code of a
//given module).
//
class OclSourceRecorder: public Intel::OpenCL::Frontend::ICLFrontendPlugin{
public:
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
  //to its corresponding source file(s)
  mutable SourceFileMap m_sourceMap;
  //lock for the source map
  mutable llvm::sys::Mutex m_sourcemapLock;
};//end class
}

#endif //__OCL_SOURCE_RECORDER_H__
