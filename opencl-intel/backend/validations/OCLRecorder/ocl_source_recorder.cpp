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

#include <assert.h>
#include "llvm/Support/MutexGuard.h"
#include "ocl_source_recorder.h"
#include "link_data.h"
#include "compile_data.h"

namespace Validation{
    using namespace Intel::OpenCL::Frontend;

    OclSourceRecorder::~OclSourceRecorder(){
    }

    void OclSourceRecorder::OnLink(const LinkData* linkData){
      //TODO: to be implemented on OCL recorder for OCL 1.2
    }

    //
    //invoked when a program is being compiled
    void OclSourceRecorder::OnCompile(const CompileData* compileData){
      assert (compileData && "NULL compileData");
      BinaryBuffer binaryBuffer = compileData->sourceFile().getBinaryBuffer();
      MD5 md5((unsigned char*)binaryBuffer.binary, binaryBuffer.size);
      MD5Code res = md5.digest();
      {
        llvm::MutexGuard lock(m_sourcemapLock);
        m_sourceMap[res] = compileData->sourceFile();
      }
      //adding the headers to the header vector
      {
        llvm::MutexGuard lock(m_headersLock);
        m_headers.insert(
          m_headers.end(),
          compileData->beginHeaders(),
          compileData->endHeaders()
        );
      }
    }
    
    FileIter OclSourceRecorder::begin(const MD5Code& code) const{
      return m_sourceMap.find(code);
    }
    
    FileIter OclSourceRecorder::end() const{
      //TODO: the implementation will change when we will insert support for CL
      // 1.2. We then need not only to return the last file, but induce the sub
      // dependency graph of the entry point module.
      FileIter ret;
      {
        llvm::MutexGuard lock(m_sourcemapLock);
        ret = m_sourceMap.end();
      }
      return ret;
    }
    //
    //CodeLess
    //
    bool CodeLess::operator () (const MD5Code& x, const MD5Code& y) const{
      const unsigned char* xcode = x.code();
      const unsigned char* ycode = y.code();
      for(int i=0 ; i<16 ; i++){
        if (xcode[i] != ycode[i])
          return xcode[i] < ycode[i];
      }
      //all the digset is actual the same one... (return false)
      return false;
    }
}
