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

#ifndef __VECTORIZER_TABLEGEN_H__
#define __VECTORIZER_TABLEGEN_H__

#include "llvm/ADT/ArrayRef.h"
#include "OclBuiltinEmitter.h"
#include <queue>
#include <map>

namespace llvm{

class VectorizerTableGen {
public:
  explicit VectorizerTableGen(RecordKeeper&);
  void run(raw_ostream&);
private:
  static const char* INVALID_ENTRY;

  typedef std::pair<bool,bool> RowProperties;
  typedef std::pair<const OclBuiltin*,std::string> BiFunction;
  typedef std::map<BiFunction, std::string> BuiltinMap;
  typedef std::queue<BiFunction> OpQueue;

  static BiFunction nullFunction();
  static bool isNullFunction(const BiFunction&);

  void processCell(const Record*);
  void generateTable(raw_ostream&, ArrayRef<RowProperties>);
 
  //maps each builtin implementation to its mangled name 
  BuiltinMap m_biMap;
  //A queue that holds the BiFunciton is their order they should be generated
  OpQueue m_opQueue;
  RecordKeeper& m_recordKeeper;
  OclBuiltinDB m_bidb;
};

}

#endif//__VECTORIZER_TABLEGEN_H__
