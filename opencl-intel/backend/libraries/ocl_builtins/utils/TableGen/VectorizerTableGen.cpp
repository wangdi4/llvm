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

#include "VectorizerTableGen.h"
#include "OclBuiltinEmitter.h"
#include "ClangUtils.h"
#include <vector>
#include <map>
#include <sstream>
#include <cstdio>
#include "llvm/TableGen/Record.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"

namespace llvm{

const char* VectorizerTableGen::INVALID_ENTRY = "INVALID_ENTRY";

VectorizerTableGen::VectorizerTableGen(RecordKeeper& rk):
  m_recordKeeper(rk), m_bidb(rk){
}

const size_t ROW_SIZE = 6;

VectorizerTableGen::BiFunction VectorizerTableGen::nullFunction(){
  return std::make_pair((OclBuiltin*)NULL, "");
}

bool
VectorizerTableGen::isNullFunction(const VectorizerTableGen::BiFunction& f){
  return f.first == NULL && f.second.empty();
}

void
VectorizerTableGen::processCell(const Record* pCell){
  assert(pCell && "null cell");
  if ("nullCell" == pCell->getNameInitAsString()){
    m_opQueue.push(nullFunction());
    return;
  }
  const Record* pBi = pCell->getValueAsDef("builtin");
  const Record* pTy = pCell->getValueAsDef("type");
  const OclBuiltin* pOclbi = m_bidb.getOclBuiltin(pBi->getNameInitAsString());
  const std::string strTy = pTy->getName();
  BiFunction biFunction = std::make_pair(pOclbi, strTy);
  //avoid future duplicated declarations
  if ( m_biMap.count(biFunction) == 0)
    m_biMap.insert(std::make_pair(biFunction, "**"));
  m_opQueue.push(biFunction);
}

void VectorizerTableGen::generateTable(raw_ostream& os,
  ArrayRef<RowProperties> arrProperties){
  size_t counter = 0;
  while (!m_opQueue.empty()){
    ++counter;
    BiFunction& biFunc = m_opQueue.front();
    if( 1 == (counter%ROW_SIZE))
      os << "{{";
    if (isNullFunction(biFunc))
      os << INVALID_ENTRY;
    else
      os << "\"" << m_biMap[biFunc] << "\" ";
    if ((counter % ROW_SIZE) != 0)
      os << ", ";
    else {
      const RowProperties& props = arrProperties[(counter-1)/ROW_SIZE];
      os << "}," << props.first << ", " << props.second << "},\n";
    }
    m_opQueue.pop();
  }
}

void VectorizerTableGen::run(raw_ostream& os){
  std::vector<Record*> mapRows =
    m_recordKeeper.getAllDerivedDefinitions("MapRow");
  const size_t numRows = mapRows.size();
  RowProperties* rowProperties = new RowProperties[numRows];
  for (size_t i=0 ; i<numRows ; ++i){
    const Record* pRow = mapRows[i];
    ArrayRef<Init*> arrCells = pRow->getValueAsListInit("cellList")->getValues();
    assert(arrCells.size() == ROW_SIZE && "cell list should have exactly six entries");
    //populating the keyes in the builtins map
    for (size_t j=0 ; j<arrCells.size() ; ++j){
      Record* pCell = dyn_cast<DefInit>(arrCells[j])->getDef();
      assert(pCell && "Record dwoncast failed");
      processCell(pCell);
    }
    //populating row propertites
    RowProperties props;
    props.first = pRow->getValueAsBit("isScalarizable");
    props.second = pRow->getValueAsBit("isPacketaizable");
    rowProperties[i] = props;
  }
  //generating an llvm module, to obtain mangled names
  std::stringstream ss;
  BuiltinMap::iterator it = m_biMap.begin(), e = m_biMap.end();
  while( it != e ) {
    const BiFunction& biFunction = it->first;
    const OclBuiltin* pOclbi = biFunction.first;
    const std::string& strTy = biFunction.second;
    std::string proto = pOclbi->getCProto(strTy);
    assert (!proto.empty() && "empty prototype?");
    ss << proto;
    std::string body = generateDummyBody(
      pOclbi->getReturnBaseCType(strTy),
      pOclbi->getReturnVectorLength(strTy)
    );
    assert (!body.empty() && "empty body?");
    ss << body;
    ss << "\n";
    ++it;
  }
  build (ss.str(), "decls.ll");
  LLVMContext context;
  llvm::SMDiagnostic errDiagnostic;
  std::unique_ptr<llvm::Module> pModule = llvm::parseIRFile("decls.ll", errDiagnostic, context);
  //we now associating each BiFunc to its corresponding llvm::Function
  it = m_biMap.begin();
  llvm::Module::const_iterator functionIt = pModule->begin(),
    functionE = pModule->end();
  while(it != e){
    it->second = functionIt->getName();
    ++it;
    ++functionIt;
  }
  assert(functionIt == functionE &&
    "the number of tblgen function and llvm functions should be the same");
  generateTable(os, ArrayRef<RowProperties>(rowProperties, numRows));
  delete[] rowProperties;
}

}//namespace
