// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __OCLBUILTINS_HEADER_GEN_H__
#define __OCLBUILTINS_HEADER_GEN_H__

#include "llvm/TableGen/Record.h"

namespace llvm {

////////////////////////////////////////////////////
//Name: OclBuiltinsHeaderGen
//Purpose: generates an array of strings, contining all the mangled names of
//the builting in the given td file.
////////////////////////////////////////////////////
class OclBuiltinsHeaderGen {
public:
  explicit OclBuiltinsHeaderGen(RecordKeeper&);
  void run(raw_ostream&);
protected:
  RecordKeeper& m_recordKeeper;
};
}
#endif//__OCLBUILTINS_HEADER_GEN_H__
