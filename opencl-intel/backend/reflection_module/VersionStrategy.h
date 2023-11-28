// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "utils.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"
#include <map>

#ifndef __VERSION_STRATEGY_H__
#define __VERSION_STRATEGY_H__

namespace Reflection {

typedef std::map<llvm::reflection::FunctionDescriptor,
                 llvm::reflection::RefParamType>
    ReturnTypeMap;

struct PairSW : std::pair<std::string, llvm::reflection::width::V> {
  PairSW(const std::pair<std::string, llvm::reflection::width::V> &);
  bool operator<(const PairSW &) const;

private:
  bool compareWild(const std::string &w, const std::string &s) const;
};

///////////////////////////////////////////////////////////////////////////////
// Purpose: VersionStrategy is an interface which returns a name of the function
// that matches a given orderd pair: (n: <mangled name>, w: <target width>).
// This is a mechnism aimes to enable functions to be versioned by a custom
// algorithm.
///////////////////////////////////////////////////////////////////////////////
struct VersionStrategy {
  virtual PairSW operator()(const PairSW &) const = 0;
  virtual ~VersionStrategy() = 0;
};

//
// Factory for the creation of null descriptors
//
struct NullDescriptorStrategy : VersionStrategy {
  PairSW operator()(const PairSW &) const override;
  ~NullDescriptorStrategy();
};

//
// AOS to SOA function descriptor conversion
//
class SoaDescriptorStrategy : public VersionStrategy,
                              public llvm::reflection::TypeVisitor {
private:
  // type synonyms
  typedef llvm::reflection::FunctionDescriptor (
      SoaDescriptorStrategy::*TransposeStrategy)(const PairSW &sw) const;

public:
  SoaDescriptorStrategy();
  void setTypeMap(const ReturnTypeMap *);
  ~SoaDescriptorStrategy();

  //////////////////////////////////////////////////////////////////////////////
  // Purpose: creates a transposed operator, with respect to the parameters
  // initialized by the 'init' method.
  // Return: the transposed function descriptor
  //////////////////////////////////////////////////////////////////////////////
  PairSW operator()(const PairSW &) const override;
  void visit(const llvm::reflection::PrimitiveType *) override;
  void visit(const llvm::reflection::VectorType *) override;
  void visit(const llvm::reflection::PointerType *) override;
  void visit(const llvm::reflection::AtomicType *) override;
  void visit(const llvm::reflection::BlockType *) override;
  void visit(const llvm::reflection::UserDefinedType *) override;

private:
  llvm::reflection::FunctionDescriptor
  scalarReturnTranspose(const PairSW &sw) const;
  llvm::reflection::FunctionDescriptor
  vectorReturnTranspose(const PairSW &sw) const;

  const ReturnTypeMap *m_pTypeMap;
  // the transpose strategy (either vector or scalar)
  TransposeStrategy m_transposeStrategy = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// "Identity strategy", which returns the pair past as parameter.
////////////////////////////////////////////////////////////////////////////////
struct IdentityStrategy : VersionStrategy {
  PairSW operator()(const PairSW &) const override;
};

std::pair<std::string, llvm::reflection::width::V>
fdToPair(const llvm::reflection::FunctionDescriptor &);

std::pair<std::string, llvm::reflection::width::V> nullPair();
bool isNullPair(const std::pair<std::string, llvm::reflection::width::V> &);

} // namespace Reflection
#endif //__VERSION_STRATEGY_H__
