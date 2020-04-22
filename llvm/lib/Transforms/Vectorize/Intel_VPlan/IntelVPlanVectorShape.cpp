//===----------------- IntelVPlanVectorShape.cpp -------------------===//
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

    if (Shape1.isUnitStride())
      return {VPVectorShape::Seq, Shape1.getStride()};
    else
      return {VPVectorShape::Str, Shape1.getStride()};
  }

  return {VPShapeDescriptor::Rnd};
}

bool VPVectorShape::shapesHaveSameStride(VPVectorShape Shape1,
                                         VPVectorShape Shape2) {
  return Shape1.hasKnownStride() && Shape2.hasKnownStride() &&
         Shape1.getStrideVal() == Shape2.getStrideVal();
}
