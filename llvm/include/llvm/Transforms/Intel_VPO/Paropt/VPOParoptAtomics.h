//===-- VPO/Paropt/VPOParoptTranform.h - Paropt Transform Class -*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Author(s):
// --------
// Abhinav Gaba (abhinav.gaba@intel.com)
//
// Major Revisions:
// ----------------
// Apr 2016: Initial Implementation supporting OMP Atomic Read (Abhinav Gaba)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains code for handling the OpenMP Atomic construct, including
/// selection as well as emitting of intrinsic calls to the KMPC runtime
/// library, based on the atomic operation and the data type(s) of the
/// operand(s).
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_ATOMICS_H
#define LLVM_TRANSFORMS_VPO_PAROPT_ATOMICS_H

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegion.h"

#include <map>

namespace llvm {

namespace vpo {

/// \brief Class used for emitting runtime calls for OpenMP atomics.
///
/// The class contains mappings to KMPC intrinsic calls for different
/// combinations of atomic operations and operands. It provides interfaces
/// to identify the operation as well as operand(s) for a given WRNAtomicNode,
/// and emit the appropriate KMPC intrinsic call. If an intrinsic call is not
/// supported for a given combination of operation and operand(s),
/// a critical section is emitted around the atomic region.
class VPOParoptAtomics {

public:
  /// \brief Main driver for handling the WRNAtomicNode \p AtomicNode.
  /// \param [in] IdentTy is the Loc struct, needed for KMPC calls.
  /// \param [in] TidPtr is pointer to the alloca instruction for
  /// thread ID. Needed for KMPC calls.
  static bool handleAtomic(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                           AllocaInst *TidPtr);

private:
  /// \name Private constructors, destructors.
  /// All methods in the class are static. Object instantiation is not needed.
  /// @{
  VPOParoptAtomics() = delete;
  ~VPOParoptAtomics() = delete;
  /// @}

  /// This is an std::pair with TypeID as first member, and size as second.
  /// This can be expanded to be a Tuple in the future if needed.
  typedef std::pair<Type::TypeID, unsigned> AtomicOperandTy;

  /// \name Atomic Intrinsic Maps.
  /// Maps are maintained to decide which call to the KMPC runtime library to
  /// make for handling a given atomic operation, on the basis of the operation
  /// and the data type(s) involved.
  /// @{

  /// \brief Map from operand Type to intrinsic function name for atomic read.
  static const std::map<AtomicOperandTy, const std::string>
      TypeToReadIntrinsicMap;

  /// \brief Map from operand Type to intrinsic function name for atomic write.
  static const std::map<AtomicOperandTy, const std::string>
      TypeToWriteIntrinsicMap;

  /// @}

  /// \name Functions for intrinsic name lookup.
  /// @{

  /// \brief Looks up the map from operand type to intrinsic names for atomic
  /// read/write. \returns the intrinsic name for the Type \p OpndTy if found,
  /// otherwise nullptr.
  template <WRNAtomicKind AtomicKind>
  static const std::string *getAtomicRWIntrinsicName(Type *OpndTy);

  /// @}

  /// \name Functions for handling different WRNAtomicNodes.
  /// @{

  /// \brief Handles Emitting of KMPC runtime calls for atomic read/write.
  /// The intrinsic calls look like:
  /// \code
  /// %4 = call i32 @__kmpc_atomic_fixed4_rd(%ident_t* %loc.addr, i32 %my.tid,
  /// i32* %opnd)
  /// call void @__kmpc_atomic_fixed4_wr(%ident_t* %loc.addr.0.05, i32 %my.tid4,
  /// i32* %temp, i32 %add)
  /// \endcode
  template <WRNAtomicKind AtomicKind>
  static bool handleAtomicRW(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                             AllocaInst *TidPtr);

  /// @}
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_ATOMICS_H
