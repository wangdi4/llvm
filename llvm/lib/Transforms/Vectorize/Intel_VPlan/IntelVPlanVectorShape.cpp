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

VPVectorShape* VPVectorShape::joinShapes(const VPVectorShape *Shape1,
                                         const VPVectorShape *Shape2) {
  if (Shape1->isUndefined())
    return new VPVectorShape(Shape2->getShapeDescriptor(), Shape2->getStride());

  if (Shape2->isUndefined())
    return new VPVectorShape(Shape1->getShapeDescriptor(), Shape1->getStride());

  if (shapesHaveSameStride(Shape1, Shape2)) {
    if (Shape1->isUniform())
      return new VPVectorShape(VPVectorShape::Uni, Shape1->getStride());
    else
      return new VPVectorShape(VPVectorShape::Str, Shape1->getStride());
  }

  return new VPVectorShape(VPVectorShape::Rnd);
}

bool VPVectorShape::shapesHaveSameStride(const VPVectorShape *Shape1,
                                         const VPVectorShape *Shape2) {
  if (Shape1 && Shape2 && Shape1->hasKnownStride() &&
      Shape2->hasKnownStride() &&
      Shape1->getStrideVal() == Shape2->getStrideVal())
    return true;
  return false;
}
