#if INTEL_COLLAB // -*- C++ -*-
//===- DebugInfoTransform.h - Debug Information Transformations -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Utilities for transforming debug information metadata within a compile unit.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_DEBUGINFOTRANSFORM_H
#define LLVM_TRANSFORMS_UTILS_DEBUGINFOTRANSFORM_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/DebugInfoMetadata.h"

namespace llvm {

class DebugInfoTransform {
private:
  DenseMap<MDNode *, MDNode *> Map; // Map to transformed metadata

public:
  DebugInfoTransform();
  virtual ~DebugInfoTransform();

  virtual bool ignore(const MDNode * const Metadata) = 0;

  // Insert an old/new metadata pair into the transformation map.
  template <class Type> void map(Type *Original, Type *Clone) {
    mapMetadata(Original, Clone);
  }

  // Clones the specified debug metadata, adds the clone to transformation map,
  // and returns the clone. The cloned metadata type must be of the same type
  // as the original.
  template <class Type> Type *clone(Type *Original) {
    return cast_or_null<Type>(cloneMetadata(Original));
  }

private:
  void mapMetadata(MDNode *original, MDNode *clone);
  MDNode *cloneMetadata(MDNode *original);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_UTILS_DEBUGINFOTRANSFORM_H

#endif // INTEL_COLLAB

