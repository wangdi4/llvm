//===-------- DDRefUtils.h - Utilities for DDRef class ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the utilities for DDRef class.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TRANSFORMS_INTEL_LOOPUTILS_DDREFUTILS_H
#define LLVM_TRANSFORMS_INTEL_LOOPUTILS_DDREFUTILS_H

namespace llvm {

namespace loopopt {

class HLNode;
class DDRef;
class RegDDRef;
class ConstDDRef;
class BlobDDRef;
class CanonExpr;

class DDRefUtils {
public:
  /// \brief Returns a new ConstDDRef.
  static ConstDDRef* createConstDDRef(CanonExpr* CE,
     HLNode* HNode = nullptr);

  /// \brief Returns a new RegDDRef.
  static RegDDRef* createRegDDRef(int SB, HLNode* HNode = nullptr);

  /// \brief Returns a new BlobDDRef.
  static BlobDDRef* createBlobDDRef(int SB, CanonExpr* CE,
    RegDDRef* Parent = nullptr);

  /// \brief Destroys the passed in DDRef.
  static void destroy(DDRef* Ref);
  /// \brief Destroys all DDRefs. Should only be called after code gen.
  static void destroyAll();
};

} // End namespace loopopt

} // End namespace llvm

#endif
