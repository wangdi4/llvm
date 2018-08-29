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

#include "ParameterType.h"
#include "FunctionDescriptor.h"
#include "utils.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/ArrayRef.h"
#include <map>

#ifndef __VERSION_STRATEGY_H__
#define __VERSION_STRATEGY_H__

namespace reflection{

typedef std::map<FunctionDescriptor, RefParamType> ReturnTypeMap;

struct PairSW : std::pair<std::string, width::V> {
  PairSW(const std::pair<std::string, width::V>&);
  bool operator < (const PairSW&)const;
  private:
  bool compareWild(const std::string& w , const std::string& s)const;
};

///////////////////////////////////////////////////////////////////////////////
//Purpose: VersionStrategy is an interface which returns a name of the function
//that matches a given orderd pair: (n: <mangled name>, w: <target width>).
//This is a mechnism aimes to enable functions to be versioned by a custom
//algorithm.
///////////////////////////////////////////////////////////////////////////////
struct VersionStrategy{
  virtual PairSW operator()(const PairSW&)const = 0;
  virtual ~VersionStrategy() = 0;
};

//
//Factory for the creation of null descriptors
//
struct NullDescriptorStrategy: VersionStrategy{
  PairSW operator()(const PairSW&)const;
  ~NullDescriptorStrategy();
};

//
//AOS to SOA function descriptor conversion
//
class SoaDescriptorStrategy: public VersionStrategy, public TypeVisitor{
private:
  //type synonyms
  typedef FunctionDescriptor
  (SoaDescriptorStrategy::*TransposeStrategy)(const PairSW& sw)const;
public:
  SoaDescriptorStrategy();
  void setTypeMap(const ReturnTypeMap*);
  ~SoaDescriptorStrategy();
  
  //////////////////////////////////////////////////////////////////////////////
  //Purpose: creates a transposed operator, with respect to the parameters
  //initialized by the 'init' method.
  //Return: the transposed function descriptor
  //////////////////////////////////////////////////////////////////////////////
  PairSW operator()(const PairSW&)const;
  void visit(const PrimitiveType*);
  void visit(const VectorType*);
  void visit(const PointerType*);
  void visit(const AtomicType*);
  void visit(const BlockType*);
  void visit(const UserDefinedType*);
private:

  FunctionDescriptor scalarReturnTranspose(const PairSW& sw)const;
  FunctionDescriptor vectorReturnTranspose(const PairSW& sw)const;
  
  const ReturnTypeMap* m_pTypeMap;
  //the transpose strategy (either vector or scalar)
  TransposeStrategy m_transposeStrategy;
};

////////////////////////////////////////////////////////////////////////////////
//Hard Coded strategy: the 'junk yard' of builtin versioning.
//When ever it is hard to apply a rule to the versioning, use this strategy as
//a way out. Note! it is our goal to keep the builtins under this strategy as
//thin as possible, since the names are hard-coded, which poses a problem when
//the mangling algorithm will change.
////////////////////////////////////////////////////////////////////////////////
class HardCodedVersionStrategy: public VersionStrategy{
public:
  //
  //Parameters:
  //  versions- an array with 6 strings, containing names of versions of the
  //  same function, as follows:
  // <v1>, <v2>, <v4> <v8>, <v16>, <v3>. Empty entries should be signaled by
  // reflection::FunctionDescriptor::nullString()
  void assumeResponsability(const TableRow*);

  PairSW operator()(const PairSW&)const;
private:
  //Maps each version to the list of containing rows in the table
  //Duplicate entries in the table are supported by using TableRowList as the container.
  //There can be duplicates in the case where there are two rows (more than two is not possible)
  //that define separate rules for scalaring and packetizing, e.g. sincos.
  typedef llvm::SmallVector<const TableRow*, 2> TableRowList;
  typedef llvm::StringMap<TableRowList> FuncName2TableRowLookup;
  FuncName2TableRowLookup m_func2row;
};

////////////////////////////////////////////////////////////////////////////////
// "Identity strategy", which returns the pair past as parameter.
////////////////////////////////////////////////////////////////////////////////
struct IdentityStrategy: VersionStrategy{
  PairSW operator()(const PairSW&)const;
};

std::pair<std::string,width::V> fdToPair(const FunctionDescriptor&);
std::pair<std::string,width::V> nullPair();
bool isNullPair(const std::pair<std::string,width::V>&);

}
#endif//__VERSION_STRATEGY_H__
