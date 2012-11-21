/****************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: VectorizerTableGen.h

\****************************************************************************/
#ifndef __VECTORIZER_TABLEGEN_H__
#define __VECTORIZER_TABLEGEN_H__

#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/ADT/ArrayRef.h"
#include "OclBuiltinEmitter.h"
#include <queue>
#include <map>

namespace llvm{

class VectorizerTableGen: public TableGenBackend {
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
