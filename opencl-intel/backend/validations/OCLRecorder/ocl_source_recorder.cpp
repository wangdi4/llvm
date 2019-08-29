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

#include <assert.h>
#include <algorithm>
#include <mutex>
#include <vector>

#include "ocl_source_recorder.h"
#include "link_data.h"
#include "compile_data.h"
#include "common_clang.h"

namespace Validation{
    using namespace Intel::OpenCL::Frontend;

    OclSourceRecorder::~OclSourceRecorder(){
    }

    void OclSourceRecorder::OnLink(const LinkData* linkData){
      //Currently we are only supporting linking of the single buffer
      if( linkData->inputBuffersCount() > 1 )
          return;

      // Nothing to do if link failed or output result not set
      if( !linkData->getBinaryResult() )
          return;

      std::vector<LinkData::BufferInfo>::const_iterator it = linkData->beginInputBuffers(), ie = linkData->endInputBuffers();
      for(;it != ie; ++it){
        unsigned char* pInputBuff = (unsigned char*)const_cast<void*>(it->first);
        MD5 md5Input(pInputBuff, it->second);
        MD5Code inputHash = md5Input.digest();

        unsigned char* pOutputBuff = (unsigned char*)const_cast<void*>(linkData->getBinaryResult()->GetIR());
        MD5 md5Output(pOutputBuff, linkData->getBinaryResult()->GetIRSize());
        MD5Code outputHash = md5Output.digest();

        {
          std::lock_guard<llvm::sys::Mutex> lock(m_sourcemapLock);
          m_sourceMap[outputHash] = m_sourceMap[inputHash];
        } 
        // get out after first iteration
        break;
      }
    }

    //
    //invoked when a program is being compiled
    void OclSourceRecorder::OnCompile(const CompileData* compileData){
      assert (compileData && "NULL compileData");
      BinaryBuffer binaryBuffer = compileData->sourceFile().getBinaryBuffer();
      MD5 md5((unsigned char*)binaryBuffer.binary, binaryBuffer.size);
      MD5Code res = md5.digest();
      SourceFilesVector sourceVector;
      sourceVector.push_back(compileData->sourceFile());
      sourceVector.insert(
        sourceVector.end(),
        compileData->beginHeaders(),
        compileData->endHeaders()
      );
      {
        std::lock_guard<llvm::sys::Mutex> lock(m_sourcemapLock);
        m_sourceMap[res] = sourceVector;
      }
    }

    FileIter OclSourceRecorder::begin(const MD5Code& code) const{
      FileIter ret;
      {
        std::lock_guard<llvm::sys::Mutex> lock(m_sourcemapLock);
        SourceFileMap::const_iterator iter = m_sourceMap.find(code);
        if (iter == m_sourceMap.end())
          return FileIter::end();
        else {
          SourceFilesVector& v = m_sourceMap[code];
          return FileIter(v.begin(), v.end());
        }
      }
    }

    FileIter OclSourceRecorder::end() const{
      //TODO: the implementation will change when we will insert support for CL
      // 1.2. We then need not only to return the last file, but induce the sub
      // dependency graph of the entry point module.
      return FileIter::end();
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
    //
    //FileIter
    //
    FileIter FileIter::end(){
      FileIter ret;
      ret.m_isExhausted = ret.m_isInitialized = true;
      return ret;
    }

    FileIter::FileIter() : m_isInitialized(false), m_isExhausted(false){
    }

    FileIter::FileIter(
      SourceFilesVector::const_iterator b,
      SourceFilesVector::const_iterator e):
      m_isInitialized(true){
      m_iter = b;
      m_iterEnd = e;
      m_isExhausted = (b == e);
    }

    bool FileIter::operator==(const FileIter& that)const{
      assert (this->m_isInitialized && that.m_isInitialized
	    && "comparission of uninitialized iterators");
      if (this == &that)
        return true;
      //one of the iterators reached the end, but not the other one
      if (this->m_isExhausted != that.m_isExhausted)
        return false;
      //both reached the end
      if (this->m_isExhausted)
        return true;
      assert (!this->m_isExhausted && !that.m_isExhausted && "internal error");
      SourceFile left = *m_iter;
      SourceFile right = *that.m_iter;
      return (left == right);
    }

    bool FileIter::operator!=(const FileIter& that)const{
      return !(this->operator==(that));
    }

    FileIter& FileIter::operator++(){
      if ( !m_isInitialized || m_iter == m_iterEnd || m_isExhausted )
        throw FileIterException();
      m_iter++;
      if (m_iter == m_iterEnd)
        m_isExhausted = true;
      return *this;
    }

    FileIter FileIter::operator++(int){
      FileIter ret(*this);
      this->operator++();
      return ret;
    }

    Intel::OpenCL::Frontend::SourceFile FileIter::operator *()const{
      return *m_iter;
    }

    const char* FileIter::FileIterException::what() const throw(){
      return "Source File iteration exception";
    }
}
