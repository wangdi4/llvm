//===------ Intel_VPlanTemplateHelper.h ------------------ -*- c++ -*------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
///
/// This files defines helpers to templatize upstream code to work with either
/// llvm::Value or vpo::VPValue type hierarchies.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_IR_INTEL_VPLAN_TEMPLATE_H
#define LLVM_IR_INTEL_VPLAN_TEMPLATE_H
#include <type_traits>

namespace llvm {

// Forward declarations.
#define VPLAN_MAP(LLVM, VPLAN) class LLVM;
#include "Intel_VPlanTemplateHelper.inc"
#undef VPLAN_MAP

namespace vpo {
#define VPLAN_MAP(LLVM, VPLAN) class VPLAN;
#include "Intel_VPlanTemplateHelper.inc"
#undef VPLAN_MAP
} // namespace vpo

struct LLVMMatcherTraits {
#define VPLAN_MAP(LLVM, VPLAN) using LLVM = llvm::LLVM;
#include "Intel_VPlanTemplateHelper.inc"
#undef VPLAN_MAP
};

struct VPlanMatcherTraits {
#define VPLAN_MAP(LLVM, VPLAN) using LLVM = vpo::VPLAN;
#include "Intel_VPlanTemplateHelper.inc"
#undef VPLAN_MAP
};

// MatcherTraitsDeducer deduces a set of basic types basing on input 'T'.
template <typename T, typename = void> struct MatcherTraitsDeducer {
  using MatcherTraits = LLVMMatcherTraits;
};

// Specialization for T, which is derived from vpo::VPValue.
template <typename T>
struct MatcherTraitsDeducer<
    T, typename std::enable_if<std::is_base_of<vpo::VPValue, T>::value>::type> {
  using MatcherTraits = VPlanMatcherTraits;
};

// Specialization to propagate MatcherTraits through Matcher types.
template <typename T>
struct MatcherTraitsDeducer<
    T,
    typename std::enable_if<std::is_same<
        typename T::MatcherTraits, typename T::MatcherTraits>::value>::type> {
  using MatcherTraits = typename T::MatcherTraits;
};

// Preprocessor can't emit new preprocessor directives so have to duplicate code
// here...
//
// Client headers are encouraged to #undef the macro once it is no longer
// needed.
#define INTEL_INJECT_VPLAN_TEMPLATIZATION(MATCHER_TYPE)                        \
  using MatcherTraits =                                                        \
      typename MatcherTraitsDeducer<MATCHER_TYPE>::MatcherTraits;              \
  using Value = typename MatcherTraits::Value;                                 \
  using Instruction = typename MatcherTraits::Instruction;                     \
  using BinaryOperator = typename MatcherTraits::BinaryOperator;               \
  using UnaryOperator = typename MatcherTraits::UnaryOperator;                 \
  using ConstantExpr = typename MatcherTraits::ConstantExpr;                   \
  using ConstantInt = typename MatcherTraits::ConstantInt;                     \
  using Operator = typename MatcherTraits::Operator;                           \
  using CallInst = typename MatcherTraits::CallInst;                           \
  using BasicBlock = typename MatcherTraits::BasicBlock;                       \
  using DominatorTree = typename MatcherTraits::DominatorTree;                 \
  using PostDominatorTree = typename MatcherTraits::PostDominatorTree;         \
  using LoopInfo = typename MatcherTraits::LoopInfo;                           \
  using Loop = typename MatcherTraits::Loop;                                   \
  using Function = typename MatcherTraits::Function
} // namespace llvm
#endif // LLVM_IR_INTEL_VPLAN_TEMPLATE_H
