//===--- HLNodeMapper.h - High level IR node mapper interface ---*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
//  The file defines HLNodeMapper interface and contains various SmallDenseMap
//  based implementations.
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDE_LLVM_IR_INTEL_LOOPIR_HLNODEMAPPER_H_
#define INCLUDE_LLVM_IR_INTEL_LOOPIR_HLNODEMAPPER_H_

#include "llvm/ADT/DenseMap.h"

namespace llvm {
namespace loopopt {

class HLNode;

class HLNodeMapper {
protected:
  virtual HLNode *getMappedImpl(const HLNode *Node) const = 0;

public:
  virtual ~HLNodeMapper() {}
  virtual void map(const HLNode *Node, HLNode *MappedNode){};

  template <typename T> T *getMapped(const T *Node) const {
    return cast_or_null<T>(getMappedImpl(Node));
  }
};

class HLNodeToNodeMapperImpl : public HLNodeMapper {
  typedef SmallDenseMap<const HLNode *, HLNode *, 16> NodeToNodeMapTy;

protected:
  NodeToNodeMapTy NodeMap;

public:
  HLNodeToNodeMapperImpl() {}
  virtual ~HLNodeToNodeMapperImpl() {}

  HLNode *getMappedImpl(const HLNode *Node) const override {
    auto Iter = NodeMap.find(Node);
    assert(Iter != NodeMap.end() && "Requesting not mapped node");
    return Iter->getSecond();
  }
};

template <typename Predicate>
class HLNodeLambdaMapperImpl : public HLNodeToNodeMapperImpl {
  Predicate Pred;

public:
  HLNodeLambdaMapperImpl(const Predicate Pred) : Pred(Pred) {}
  virtual ~HLNodeLambdaMapperImpl() {}

  void map(const HLNode *Node, HLNode *MappedNode) override {
    if (Pred(Node)) {
      NodeMap[Node] = MappedNode;
    }
  }
};

namespace HLNodeLambdaMapper {

template <typename Predicate>
HLNodeLambdaMapperImpl<Predicate> mapper(Predicate Pred) {
  return HLNodeLambdaMapperImpl<Predicate>(Pred);
}
}

struct HLNodeToNodeMapper final : public HLNodeToNodeMapperImpl {
  HLNodeToNodeMapper() {}
  virtual ~HLNodeToNodeMapper() {}

  template <typename... T>
  HLNodeToNodeMapper(T... Args) {
    auto x = { (add(Args), 0)... };
    (void)x;
  }

  void map(const HLNode *Node, HLNode *MappedNode) override {
    auto Iter = NodeMap.find(Node);
    if (Iter != NodeMap.end()) {
      Iter->getSecond() = MappedNode;
    }
  }

  void add(const HLNode *Node) {
    NodeMap.insert(std::make_pair(Node, nullptr));
  }
};
}
}

#endif /* INCLUDE_LLVM_IR_INTEL_LOOPIR_HLNODEMAPPER_H_ */
