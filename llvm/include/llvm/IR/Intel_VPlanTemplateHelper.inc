// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
// No include guard
VPLAN_MAP(Value, VPValue)
VPLAN_MAP(Instruction, VPInstruction)
VPLAN_MAP(ICmpInst, VPCmpInst)
VPLAN_MAP(ConstantInt, VPConstantInt)
// TODO:
// *Operator and ConstantExpr types need fine grained counterparts in VPlan.
VPLAN_MAP(BinaryOperator, VPInstruction)
VPLAN_MAP(UnaryOperator, VPInstruction)
VPLAN_MAP(Operator, VPInstruction)
VPLAN_MAP(ConstantExpr, VPInstruction)
VPLAN_MAP(CallInst, VPCallInstruction)
VPLAN_MAP(BasicBlock, VPBasicBlock)
VPLAN_MAP(DominatorTree, VPDominatorTree)
VPLAN_MAP(PostDominatorTree, VPPostDominatorTree)
VPLAN_MAP(LoopInfo, VPLoopInfo)
VPLAN_MAP(Loop, VPLoop)
VPLAN_MAP(Function, VPlan)
