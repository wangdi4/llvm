//===- BranchPredictionModel.cpp - Branch Prediction Model ------*- C++ -*-===//
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

#include "BranchPredictionModel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.h"

#include <cassert>
#include <cmath>

#define DEBUG_TYPE "mlpgo"

namespace machine_learning_engine {

std::string BranchPredictionModel::getModelFileName() {
  return "icx_bp_0.1.1.onnx";
}
MlModelStatus
BranchPredictionModel::predict(double *prediction,
                               const llvm::mlpgo::MLBrFeatureVec &FeatureVec) {
  try {
    *prediction = NOPRED;

    assert(FeatureVec.getNumOfSucc() == 2);
    const llvm::mlpgo::BrSrcBBFeturesT &SrcBBFeatures =
        FeatureVec.getSrcBBFeatures();

    const llvm::mlpgo::BrSuccFeaturesT &LeftSuccFeatures =
        FeatureVec.getSuccFeatures(0);
    const llvm::mlpgo::BrSuccFeaturesT &RightSuccFeatures =
        FeatureVec.getSuccFeatures(1);

    // Note: it is crucial that the features will be placed in the vector in the
    // exact order expected by the model
    std::vector<float> input{
        (float)SrcBBFeatures.srcBranchPredicate,
        (float)SrcBBFeatures.srcBranchOperandOpcode,
        (float)SrcBBFeatures.srcBranchOperandFunc,
        (float)SrcBBFeatures.srcBranchOperandType,
        (float)SrcBBFeatures.srcRAOpCode,
        (float)SrcBBFeatures.srcRAFunc,
        (float)SrcBBFeatures.srcRAType,
        (float)SrcBBFeatures.srcRBOpCode,
        (float)SrcBBFeatures.srcRBFunc,
        (float)SrcBBFeatures.srcRBType,
        (float)SrcBBFeatures.srcLoopHeader,
        (float)SrcBBFeatures.srcProcedureType,
        (float)SrcBBFeatures.srcLoopDepth,
        (float)SrcBBFeatures.srcLoopBlockSize,
        (float)SrcBBFeatures.srcTotalSubLoopSize,
        (float)SrcBBFeatures.srcTotalSubLoopBlockSize,
        (float)SrcBBFeatures.srcLoopExitingSize,
        (float)SrcBBFeatures.srcLoopExitSize,
        (float)SrcBBFeatures.srcLoopExitEdgesSize,
        (float)SrcBBFeatures.srcTriangle,
        (float)SrcBBFeatures.srcDiamond,
        (float)SrcBBFeatures.srcFunctionStartWithRet,
        (float)SrcBBFeatures.srcFunctionInstructionSize,
        (float)SrcBBFeatures.srcFunctionBlockSize,
        (float)SrcBBFeatures.srcFunctionEdgesSize,
        (float)SrcBBFeatures.srcNumberOfSuccessors,

        (float)LeftSuccFeatures.SuccessorsRank,
        (float)LeftSuccFeatures.SuccessorBranchDirection,
        (float)LeftSuccFeatures.SuccessorLoopHeader,
        (float)LeftSuccFeatures.SuccesorLoopBack,
        (float)LeftSuccFeatures.SuccessorExitEdge,
        (float)LeftSuccFeatures.SuccessorsCall,
        (float)LeftSuccFeatures.SuccessorsEnd,
        (float)LeftSuccFeatures.SuccessorsUseDef,
        (float)LeftSuccFeatures.SuccessorBranchDominate,
        (float)LeftSuccFeatures.SuccessorsBranchPostDominate,
        (float)LeftSuccFeatures.SuccessorUnlikely,
        (float)LeftSuccFeatures.SuccessorNumberOfSiblingExitSuccessors,
        (float)LeftSuccFeatures.SuccessorEstimatedWeight,
        (float)LeftSuccFeatures.SuccessorTotalWeight,
        (float)LeftSuccFeatures.SuccessorInstructionSize,
        (float)LeftSuccFeatures.SuccessorStore,

        (float)RightSuccFeatures.SuccessorsRank,
        (float)RightSuccFeatures.SuccessorBranchDirection,
        (float)RightSuccFeatures.SuccessorLoopHeader,
        (float)RightSuccFeatures.SuccesorLoopBack,
        (float)RightSuccFeatures.SuccessorExitEdge,
        (float)RightSuccFeatures.SuccessorsCall,
        (float)RightSuccFeatures.SuccessorsEnd,
        (float)RightSuccFeatures.SuccessorsUseDef,
        (float)RightSuccFeatures.SuccessorBranchDominate,
        (float)RightSuccFeatures.SuccessorsBranchPostDominate,
        (float)RightSuccFeatures.SuccessorUnlikely,
        (float)RightSuccFeatures.SuccessorNumberOfSiblingExitSuccessors,
        (float)RightSuccFeatures.SuccessorEstimatedWeight,
        (float)RightSuccFeatures.SuccessorTotalWeight,
        (float)RightSuccFeatures.SuccessorInstructionSize,
        (float)RightSuccFeatures.SuccessorStore,
    };

    std::vector<float> output;

    // TODO: validate output size is as expected. probably initialize() should
    // keep output size read from onnx as member of this class

    this->runModel(input, output);

    LLVM_DEBUG(llvm::dbgs() << "Branch Prediction Model result is: "
                            << std::to_string(output[0]) << "\n");

    *prediction = output[0];

    return MlModelStatus::OK;
  }

  catch (...) {
    LLVM_DEBUG(llvm::dbgs() << "Failed to execute the model\n");
    return MlModelStatus::GENERAL_ERROR;
  }
}

} // namespace machine_learning_engine
