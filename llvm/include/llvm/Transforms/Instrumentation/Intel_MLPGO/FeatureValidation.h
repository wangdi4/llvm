//===- FeatureValidation.h - Feature Validation -----------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_MLPGO_INTEL_FEATUREVALIDATION_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_MLPGO_INTEL_FEATUREVALIDATION_H

#include "llvm/ADT/ArrayRef.h"

#include <array>
#include <unordered_map>
#include <utility>

using std::array;
using std::pair;
using std::unordered_map;
namespace llvm {
namespace mlpgo {

/* feature validation */
/* enumerate common features */
enum class SrcBBFeatures : unsigned {
#define BR_SRC_BB_FEATURE(Name) Name,
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SRC_BB_FEATURE
  SrcBBFeaturesSize,
};

/* enumerate successor features */
enum class SuccFeatures : unsigned {
#define BR_SUCC_BB_FEATURE(Name) Name,
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SUCC_BB_FEATURE
  SuccFeaturesSize,
};

enum class ValidationType : unsigned {
  ENUM,
  RANGE,
};

constexpr array PredValues = {
    static_cast<unsigned>(CmpInst::Predicate::FCMP_FALSE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_OEQ),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_OGT),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_OGE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_OLT),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_OLE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_ONE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_ORD),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_UNO),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_UEQ),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_UGT),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_UGE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_ULT),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_ULE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_UNE),
    static_cast<unsigned>(CmpInst::Predicate::FCMP_TRUE),          // 15
    static_cast<unsigned>(CmpInst::Predicate::BAD_FCMP_PREDICATE), // 16
    static_cast<unsigned>(CmpInst::Predicate::ICMP_EQ),            // 32
    static_cast<unsigned>(CmpInst::Predicate::ICMP_NE),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_UGT),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_UGE),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_ULT),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_ULE),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_SGT),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_SGE),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_SLT),
    static_cast<unsigned>(CmpInst::Predicate::ICMP_SLE),          // 41
    static_cast<unsigned>(CmpInst::Predicate::BAD_ICMP_PREDICATE) // 42
};

constexpr array OpCodeValues = {
    // only invoke is possible among terminate instructions.
    static_cast<unsigned>(Instruction::Invoke),

#define FIRST_UNARY_INST(N)
#define HANDLE_UNARY_INST(N, OPC, CLASS)                                       \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_UNARY_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_BINARY_INST(N)
#define HANDLE_BINARY_INST(N, OPC, CLASS)                                      \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_BINARY_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_MEMORY_INST(N)
#define HANDLE_MEMORY_INST(N, OPC, CLASS)                                      \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_MEMORY_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_CAST_INST(N)
#define HANDLE_CAST_INST(N, OPC, CLASS) static_cast<unsigned>(Instruction::OPC),
#define LAST_CAST_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_FUNCLETPAD_INST(N)
#define HANDLE_FUNCLETPAD_INST(N, OPC, CLASS)                                  \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_FUNCLETPAD_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_OTHER_INST(N)
#define HANDLE_OTHER_INST(N, OPC, CLASS)                                       \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_OTHER_INST(N)
#include "llvm/IR/Instruction.def"

    static_cast<unsigned>(ExtendOpFunc::BadOpFunc),

};

constexpr array OpFuncValues = {
    // only invoke is possible among terminate instructions.
    static_cast<unsigned>(Instruction::Invoke),

#define FIRST_UNARY_INST(N)
#define HANDLE_UNARY_INST(N, OPC, CLASS)                                       \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_UNARY_INST(N)
#include "llvm/IR/Instruction.def"

    static_cast<unsigned>(Instruction::Add),
    static_cast<unsigned>(Instruction::Sub),
    static_cast<unsigned>(Instruction::Mul),
    static_cast<unsigned>(Instruction::FMul),
    static_cast<unsigned>(Instruction::UDiv),
    static_cast<unsigned>(Instruction::URem),

    static_cast<unsigned>(Instruction::Shl),
    static_cast<unsigned>(Instruction::LShr),
    static_cast<unsigned>(Instruction::AShr),
    static_cast<unsigned>(Instruction::And),
    static_cast<unsigned>(Instruction::Or),
    static_cast<unsigned>(Instruction::Xor),

#define FIRST_MEMORY_INST(N)
#define HANDLE_MEMORY_INST(N, OPC, CLASS)                                      \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_MEMORY_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_CAST_INST(N)
#define HANDLE_CAST_INST(N, OPC, CLASS) static_cast<unsigned>(Instruction::OPC),
#define LAST_CAST_INST(N)
#include "llvm/IR/Instruction.def"

#define FIRST_FUNCLETPAD_INST(N)
#define HANDLE_FUNCLETPAD_INST(N, OPC, CLASS)                                  \
  static_cast<unsigned>(Instruction::OPC),
#define LAST_FUNCLETPAD_INST(N)
#include "llvm/IR/Instruction.def"

    static_cast<unsigned>(Instruction::ICmp),
    static_cast<unsigned>(Instruction::PHI),
    static_cast<unsigned>(Instruction::Call),
    static_cast<unsigned>(Instruction::Select),
    static_cast<unsigned>(Instruction::UserOp1),
    static_cast<unsigned>(Instruction::UserOp2),
    static_cast<unsigned>(Instruction::VAArg),
    static_cast<unsigned>(Instruction::ExtractElement),
    static_cast<unsigned>(Instruction::InsertElement),
    static_cast<unsigned>(Instruction::ShuffleVector),
    static_cast<unsigned>(Instruction::ExtractValue),
    static_cast<unsigned>(Instruction::InsertValue),
    static_cast<unsigned>(Instruction::LandingPad),
    static_cast<unsigned>(Instruction::Freeze),

    static_cast<unsigned>(ExtendOpFunc::BadOpFunc), // extend
    static_cast<unsigned>(ExtendOpFunc::ConstantZeroOpFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantOpFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantOneFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantMinusOneFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantOnlyZeroFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantOnlyOneFunc),
    static_cast<unsigned>(ExtendOpFunc::ConstantOnlyMinusOneFunc),
    static_cast<unsigned>(ExtendOpFunc::LoadGlobalOpFunc),

};

constexpr array OpTypeRanges = {
    static_cast<unsigned>(TypeDescIdx::HalfTy),
    static_cast<unsigned>(TypeDescIdx::LabelTy),
};

constexpr array BoolValues = {
    static_cast<unsigned>(0),
    static_cast<unsigned>(1),
};

constexpr array ProcedureTypeValues = {
    static_cast<unsigned>(ProcedureType::Leaf),
    static_cast<unsigned>(ProcedureType::NonLeaf),
    static_cast<unsigned>(ProcedureType::CallSelf),
    static_cast<unsigned>(ProcedureType::OnlyIntrinsic),
};

constexpr array BranchDirectionValues = {
    static_cast<unsigned>(BranchDirection::Forward),
    static_cast<unsigned>(BranchDirection::Backward),
};

constexpr array SuccessorExitEdgeValues = {
    static_cast<unsigned>(SuccessorExitEdgeType::NLE),
    static_cast<unsigned>(SuccessorExitEdgeType::LE),
    static_cast<unsigned>(SuccessorExitEdgeType::SiblingLE),
};

constexpr array SuccessorUnlikelyValues = {
    static_cast<unsigned>(SuccessorUnlikelyType::Normallikely),
    static_cast<unsigned>(SuccessorUnlikelyType::Unlikely),
    static_cast<unsigned>(SuccessorUnlikelyType::SiblingUnlikely),
};

constexpr array SuccessorsEndValues = {
    static_cast<unsigned>(SuccessorEndKind::Nothing),
    static_cast<unsigned>(SuccessorEndKind::FT),
    static_cast<unsigned>(SuccessorEndKind::CBR),
    static_cast<unsigned>(SuccessorEndKind::SWITCH),
    static_cast<unsigned>(SuccessorEndKind::IVK),
    static_cast<unsigned>(SuccessorEndKind::UBR),
    static_cast<unsigned>(SuccessorEndKind::IJUMP),
    static_cast<unsigned>(SuccessorEndKind::IJSR),
    static_cast<unsigned>(SuccessorEndKind::Ret),
    static_cast<unsigned>(SuccessorEndKind::Resume),
};

constexpr array SrcBBFeaturesValuesMap = {
    pair(static_cast<unsigned>(SrcBBFeatures::srcBranchPredicate),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*PredValues.begin(), PredValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcBranchOperandOpcode),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpCodeValues.begin(), OpCodeValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcBranchOperandFunc),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpFuncValues.begin(), OpFuncValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcBranchOperandType),
         pair(static_cast<unsigned>(ValidationType::RANGE),
              ArrayRef<unsigned>(&*OpTypeRanges.begin(), OpTypeRanges.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRAOpCode),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpCodeValues.begin(), OpCodeValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRAFunc),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpFuncValues.begin(), OpFuncValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRAType),
         pair(static_cast<unsigned>(ValidationType::RANGE),
              ArrayRef<unsigned>(&*OpTypeRanges.begin(), OpTypeRanges.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRBOpCode),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpCodeValues.begin(), OpCodeValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRBFunc),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*OpFuncValues.begin(), OpFuncValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcRBType),
         pair(static_cast<unsigned>(ValidationType::RANGE),
              ArrayRef<unsigned>(&*OpTypeRanges.begin(), OpTypeRanges.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcLoopHeader),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcProcedureType),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*ProcedureTypeValues.begin(),
                                 ProcedureTypeValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcTriangle),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcDiamond),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SrcBBFeatures::srcFunctionStartWithRet),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),

};

constexpr array SuccFeaturesValuesMap = {
    pair(static_cast<unsigned>(SuccFeatures::SuccessorBranchDirection),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BranchDirectionValues.begin(),
                                 BranchDirectionValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorLoopHeader),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccesorLoopBack),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorExitEdge),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*SuccessorExitEdgeValues.begin(),
                                 SuccessorExitEdgeValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorsCall),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorsEnd),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*SuccessorsEndValues.begin(),
                                 SuccessorsEndValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorsUseDef),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorBranchDominate),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorsBranchPostDominate),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorUnlikely),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*SuccessorUnlikelyValues.begin(),
                                 SuccessorUnlikelyValues.size()))),
    pair(static_cast<unsigned>(SuccFeatures::SuccessorStore),
         pair(static_cast<unsigned>(ValidationType::ENUM),
              ArrayRef<unsigned>(&*BoolValues.begin(), BoolValues.size()))),
};

} // end namespace mlpgo

} // end namespace llvm

#endif
