//===----------------- IntelVPlanVectorShape.cpp -------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
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
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements utility functions for Vector Shapes.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlan.h"
#include "IntelVPlanVectorShape.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-vector-shape"

VPVectorShape VPVectorShape::joinShapes(VPVectorShape Shape1,
                                        VPVectorShape Shape2) {
  if (Shape1.isUndefined())
    return {Shape2.getShapeDescriptor(), Shape2.getStride()};

  if (Shape2.isUndefined())
    return {Shape1.getShapeDescriptor(), Shape1.getStride()};

  if (shapesHaveSameStride(Shape1, Shape2)) {
    if (Shape1.isUniform())
      return {VPVectorShape::Uni, Shape1.getStride()};

    if (Shape1.isSOAUnitStride())
      return {VPVectorShape::SOASeq, Shape1.getStride()};
    if (Shape1.isUnitStride())
      return {VPVectorShape::Seq, Shape1.getStride()};
    else
      return {VPVectorShape::Str, Shape1.getStride()};
  }

  // We want to return SOARand for the following inputs
  // Shape1 = SOAStr, Shape2 = SOASeq
  // Shape1 = SOASeq, Shape2 = SOAStr
  // Shape1 = SOAStr, Shape2 = SOARnd
  // ....
  // i.e., if either of Shape1 or Shape2 is not SOASeq,
  // we want to return SOARnd.
  // The case where Shape1 is SOASeq and Shape2 is SOASeq is handled in the
  // previous block.
  if (Shape1.isSOAShape() && Shape2.isSOAShape())
    return {VPShapeDescriptor::SOARnd};

  return {VPShapeDescriptor::Rnd};
}

bool VPVectorShape::shapesHaveSameStride(VPVectorShape Shape1,
                                         VPVectorShape Shape2) {
  // Shapes have to be of the same class, i.e., SOA or non-SOA.
  // It is possible that we encounter SOA UnitStride and UnitStride
  // with the same stride. We return 'false' in such cases.
  return Shape1.isSOAShape() == Shape2.isSOAShape() &&
         Shape1.hasKnownStride() && Shape2.hasKnownStride() &&
         Shape1.getStrideVal() == Shape2.getStrideVal();
}
