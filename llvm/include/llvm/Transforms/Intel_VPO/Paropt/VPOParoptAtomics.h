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
  /// This is a pair with an Instruction Op-code as the first member, and a pair
  /// of AtomicOperandTy's as the second member.
  typedef std::pair<unsigned, std::pair<AtomicOperandTy, AtomicOperandTy>>
      AtomicOperationTy;

  /// \name Instances of AtomicOperadnTy objects of different kinds.
  /// @{
  static const AtomicOperandTy I8, I16, I32, I64;
  static const AtomicOperandTy P32, P64;
  static const AtomicOperandTy F32, F64, F80, F128;
  /// @}

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

  /// \brief Map from operation and operand types to intrinsic name for atomic
  /// update.
  static const std::map<AtomicOperationTy, const std::string>
      OpToUpdateIntrinsicMap;

  /// \brief Map from operation type to intrinsic name for atomic update, when
  /// the atomic operand and the value operand are reversed.
  /// For example: atomic_opnd = value_opnd - atomic_opnd
  static const std::map<AtomicOperationTy, const std::string>
      ReversedOpToUpdateIntrinsicMap;

  /// @}

  /// \name Functions for intrinsic name lookup.
  /// @{

  /// \brief Looks up the map from operand type to intrinsic name for
  /// atomic read/write.
  /// \param OpInst is the load/store instruction corresponding to atomic
  /// read/write.
  /// \returns the intrinsic name for the Type \p OpndTy if
  /// found, otherwise an empty string.
  template <WRNAtomicKind AtomicKind>
  static const std::string getAtomicRWIntrinsicName(Instruction *OpInst,
                                                    Type *OpndTy);

  /// \brief Looks up the map from operation type to intrinsic name for
  /// atomic update. Consider the following two examples of atomic update:
  /// \code
  ///      x = x - expr; // (1)
  ///      y = val - y;  // (2)
  /// \endcode
  /// \param Op is the LLVM Instruction which represents the binary operation,
  /// for example, if the operands above are integers, Op will be a `sub` here.
  /// \param AtomicOpnd is the operand `x` in (1), and `y` in (2).
  /// \param ValueOpnd is the operand 'expr' in (1), and 'val' in (2).
  /// \param Reversed specifies if in case of a non-commutative binary
  /// operation, such as `-` in (1) and (2), the second operand is the atomic
  /// operand. So, Reversed is `false` for (1) and `true` for (2).
  /// Note: \p AtomicOpnd and \p ValueOpnd are not the actual operands of the
  /// LLVM instruction \p Op. For example, if in (1), `x` is an unsigned
  /// integer, and `y` a float, then the IR will be like:
  /// \code
  ///     %2 = load float, float* %val, align 4
  ///     %3 = load i32, i32* %y, align 4
  ///     %conv = uitofp i32 %3 to float
  ///     %sub = fsub float %2, %conv
  ///     %conv1 = fptoui float %sub to i32
  ///     store i32 %conv1, i32* %y, align 4
  /// \endcode
  /// Here, \p Op is the Instruction `%sub`, \p AtomicOpnd is %y and \p
  /// ValueOpnd is %2.
  /// \returns Intrinsic name for the operation, if found, otherwise an empty
  /// string.
  static const std::string getAtomicUpdateIntrinsicName(bool Reversed,
                                                        Instruction *Op,
                                                        Value *AtomicOpnd,
                                                        Value *ValueOpnd);

  /// \brief Adjusts \p IntrinsicName for non-X86 architectures.
  /// The function looks for the substring '_a16' in \p IntrinsicName, removes
  /// it from the name, and \p returns the resulting string for non X64
  /// architectures.
  /// \param Inst is any instruction from the module in which the Intrinsic is
  /// to be inserted. It is used to get the architecture information from the
  /// module it belongs to.
  /// TODO: Use TableGen for creating the tables.
  static const std::string
  adjustIntrinsicNameForArchitecture(Instruction *Inst,
                                     const std::string &IntrinsinName);

  /// @}

  /// \name Functions for handling different WRNAtomicNodes.
  /// @{

  /// \brief Handles Emitting of KMPC runtime calls for atomic read/write.
  /// For atomic read/write, the incoming Load/Store instructions are of form:
  ///     <ret> = load <type*> atomic_opnd
  ///     store <type> value_opnd, <type*> atomic_opnd
  /// And the KMPC atomic read/write calls are of form:
  ///     <ret> = call __kmpc_atomic_<dtype>_rd(loc, tid, atomic_opnd)
  ///     call __kmpc_atomic_<dtype>_wr(loc, tid, atomic_opnd, value_opnd)
  /// Example:
  /// \code
  ///     %7 = call i32 @__kmpc_atomic_fixed4_rd(%ident_t* %loc.addr, i32
  ///     %my.tid, i32* %atomic_opnd)
  ///     call void @__kmpc_atomic_fixed4_wr(%ident_t* %loc.addr, i32 %my.tid,
  ///     i32* %atomic_opnd, i32 %val)
  /// \endcode
  template <WRNAtomicKind AtomicKind>
  static bool handleAtomicRW(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                             AllocaInst *TidPtr);

  /// \brief Handles Emitting of KMPC runtime calls for atomic update.
  /// For an incoming IR of form:
  ///     A = load atomic_opnd
  ///     result = <operation> A, value_opnd
  ///     store result, atomic_opnd
  /// The KMPC call generated for atomic update is of form:
  ///     call void __kmpc_atomic_<op-type>(loc, tid, <type1*> atomic_opnd,
  ///     <type2> value_opnd)
  /// Example:
  /// \code
  ///     call void @__kmpc_atomic_fixed4_add_fp(%ident_t* %loc.addr, i32
  ///     %my.tid, i32* %atomic_opnd, fp128 %fpext.other.opnd)
  /// \endcode
  static bool handleAtomicUpdate(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                                 AllocaInst *TidPtr);

  /// @}

  /// \name Private Helper Methods.
  /// @{

  /// \brief Generates FPExt cast for \p ValueOpnd in case of mixed atomic
  /// update with integer \p AtomicOpnd and floating point \p ValueOpnd, if
  /// needed. If \p AtomicOpnd is an integer, and \p ValueOpnd is a float such
  /// that an intrinsic for the combination  would not exist, then a CastInst is
  /// generated to extend \p ValueOpnd to a higher-precision floating point
  /// value. \p Op is needed to obtain the target size of the FPExt cast.
  /// \returns the FPExt CastInst for \p ValueOpnd, if supported/needed,
  /// `nullptr` otherwise.
  static CastInst *genFPExtForNonAtomicOpnd(Instruction *Op,
                                            Value *AtomicOpnd,
                                            Value *ValueOpnd);
  /// \brief Returns `true` if Val is a UIToFP CastInst, `false` otherwise.
  static bool isUIToFPCast(Value *Val);

  /// @}
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_ATOMICS_H
