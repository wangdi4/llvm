//===-- VPO/Paropt/VPOParoptAtomics.h - Paropt Atomics Class -*- C++ -*-===//
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
// Apr 2016: Initial Implementation for OMP Atomic Read, Write. (Abhinav Gaba)
// May 2016: Added support for OMP Atomic Update. (Abhinav Gaba)
// Jun 2016: Added support for OMP Atomic Capture (Abhinav Gaba).
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
  /// \param [in] AtomicNode is the WRNAtomicNode for which the KMPC calls are
  /// to be generated.
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

  /// Enum for different types of Atomic Capture operations.
  enum AtomicCaptureKind {
    CaptureUnknown = -1,
    CaptureAfterOp,  ///< v = x = x op expr; or {x = x op expr; v = x;}
    CaptureBeforeOp, ///< {v = x; x = x op expr;}
    CaptureSwap      ///< {v = x; x = expr;}
  };

  /// \name Instances of AtomicOperadnTy objects of different kinds.
  /// @{
  static const AtomicOperandTy I8, I16, I32, I64;
  static const AtomicOperandTy P32, P64;
  static const AtomicOperandTy F32, F64, F80, F128;
  /// @}

  /// \name Atomic Intrinsic Maps.
  /// Maps are maintained to decide which call to the KMPC runtime library to
  /// make for handling a given atomic operation, on the basis of the
  /// operation and the data type(s) involved.
  /// @{

  /// \brief Map from operand Type to intrinsic name for atomic read.
  static const std::map<AtomicOperandTy, const std::string>
      TypeToReadIntrinsicMap;

  /// \brief Map from operand Type to intrinsic name for atomic write.
  static const std::map<AtomicOperandTy, const std::string>
      TypeToWriteIntrinsicMap;

  /// \brief Map from operand Type to intrinsic name for atomic capture swap.
  static const std::map<AtomicOperandTy, const std::string>
      TypeToSwapIntrinsicMap;

  /// \brief Map from operation and operand types to intrinsic name for atomic
  /// update.
  static const std::map<AtomicOperationTy, const std::string>
      OpToUpdateIntrinsicMap;

  /// \brief Map from operation type to intrinsic name for atomic update, when
  /// the atomic operand and the value operand are reversed.
  /// For example: atomic_opnd = value_opnd - atomic_opnd
  static const std::map<AtomicOperationTy, const std::string>
      ReversedOpToUpdateIntrinsicMap;

  /// \brief Map from operation and operand types to intrinsic name for atomic
  /// capture.
  static const std::map<AtomicOperationTy, const std::string>
      OpToCaptureIntrinsicMap;

  /// \brief Map from operation type to intrinsic name for atomic capture,
  /// when the atomic operand and the value operand are reversed.
  /// For example: capture_opnd = atomic_opnd = value_opnd - atomic_opnd
  static const std::map<AtomicOperationTy, const std::string>
      ReversedOpToCaptureIntrinsicMap;

  /// @}

  /// \name Functions for handling different WRNAtomicNodes.
  /// @{

  /// \brief Handles Emitting of KMPC runtime calls for atomic read/write.
  /// For atomic read/write, the incoming Load/Store instructions are of form:
  ///     <ret> = load <type*> atomic_opnd
  ///     store <type> value_opnd, <type*> atomic_opnd
  ///
  /// And the KMPC atomic read/write calls are of form:
  ///     <ret> = call __kmpc_atomic_<dtype>_rd(loc, tid, atomic_opnd)
  ///     call __kmpc_atomic_<dtype>_wr(loc, tid, atomic_opnd, value_opnd)
  ///
  /// Example:
  /// \code
  ///     %7 = call i32 @__kmpc_atomic_fixed4_rd(%ident_t* %loc.addr, i32
  ///     %my.tid, i32* %atomic_opnd)
  ///     call void @__kmpc_atomic_fixed4_wr(%ident_t* %loc.addr, i32 %my.tid,
  ///     i32* %atomic_opnd, i32 %val)
  /// \endcode
  ///
  /// \returns `true` if \p AtomicNode was successfully handled, `false`
  /// otherwise.
  template <WRNAtomicKind AtomicKind>
  static bool handleAtomicRW(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                             AllocaInst *TidPtr);

  /// \brief Handles Emitting of KMPC runtime calls for atomic update.
  /// For an incoming IR of form:
  ///     A = load atomic_opnd
  ///     result = <operation> A, value_opnd
  ///     store result, atomic_opnd
  ///
  /// The KMPC call generated for atomic update is of form:
  ///     call void __kmpc_atomic_<op-type>(loc, tid, <type1*> atomic_opnd,
  ///     <type2> value_opnd)
  ///
  /// Example of KMPC call:
  /// \code
  ///     call void @__kmpc_atomic_fixed4_add_fp(%ident_t* %loc.addr, i32
  ///     %my.tid, i32* %atomic_opnd, fp128 %fpext.other.opnd)
  /// \endcode
  ///
  /// \returns `true` if \p AtomicNode was successfully handled, `false`
  /// otherwise.
  static bool handleAtomicUpdate(WRNAtomicNode *AtomicNode, StructType *IdentTy,
                                 AllocaInst *TidPtr);

  /// \brief Handles Emitting of KMPC runtime calls for atomic capture.
  /// For atomic_opnd `x`, capture_opnd `v` and value_opnd `expr`, the capture
  /// operation can be of one of the three forms:
  /// (i)   Capture before op: {v = x; <update operation>;}
  /// (ii)  Capture after op: {<update operation>; v = x;} or v = <update
  ///       operation>;
  /// (iii) Capture-swap: {v = x; x = expr;}
  /// where <update operation> can be of form:
  ///       (a) x = x op expr
  ///       (b) x = expr op x
  ///       (c) x op= expr
  ///
  /// The generated code is of form:
  ///     %1 = call <type1> __kmpc_atomic_<...>_cpt(loc, tid, <type1>*
  ///          atomic_opnd, <type2> value_opnd[, capture_flag])
  ///     %cpt.opnd.cast = <cast> <type3> %1
  ///     store <type3> %cpt.opnd.cast, <type3>* capture_opnd
  ///
  /// Example of KMPC call:
  /// \code
  ///     %3 = call float @__kmpc_atomic_float4_sub_cpt(%ident_t*
  ///     %loc.addr.0.01, i32 %my.tid, float* %x, float %val.opnd, i32 1)
  /// \endcode
  ///
  /// \returns `true` if \p AtomicNode was successfully handled, `false`
  /// otherwise.
  static bool handleAtomicCapture(WRNAtomicNode *AtomicNode,
                                  StructType *IdentTy, AllocaInst *TidPtr);

  /// @}

  /// \name Methods for identifying the op and opnds for Atomic update and
  /// capture.
  /// @{

  /// \brief Using \p AtomicOpnd as the atomic operand for atomic update,
  /// finds the operation as well as the value operand from \p BB.
  ///
  /// Example: Consider the update operations:
  ///     x = expr / x;
  ///
  /// IR:
  /// \code
  ///    %2 = load double, double* %expr, align 8
  ///    %3 = load i32, i32* %x, align 4       ; <- (i)
  ///    %conv = uitofp i32 %3 to double       ; <- (ii)
  ///    %div = fdiv double %2, %conv          ; <- (iii)
  ///    %conv1 = fptoui double %div to i32    ; <- (iv)
  ///    store i32 %conv1, i32* %x, align 4    ; <- (v)
  /// \endcode
  ///
  /// \param [in] BB The middle BasicBlock of the WRNAtomicNode.
  /// \param [in] AtomicOpnd is the atomic operand. (%x here)
  /// \param [out] OpInst is the instruction where the result being stored to
  /// the \p AtomicOpnd is calculated (before any casts). (%div here)
  /// \param [out] ValueOpnd is the operand to the update operation, other
  /// than \p AtomicOpnd. (%2 here)
  /// \param [out] Reversed is set to `true`, if for a non-commutative
  /// operation like sub/div, the atomic operand is the second one.
  /// e.g. `x = v - x`, where `x` is the atomic operand. (`true` here)
  /// \param [out] AtomicOpndStore is the StoreInst storing the result of \p
  /// OpInst to \p AtomicOpnd. (Instruction (v) here)
  /// \param [in,out] InstsToDelete will be populated with Instructions that
  /// will need to be deleted if a KMPC intrinsic call is inserted for the
  /// atomic operation (also marked in the IR above):
  ///   (i) the load of AtomicOpnd,
  ///   (ii) CastInst from that loaded value to the operand of OpInst,
  ///   (iii) OpInst itself, and
  ///   (iv) The CastInst from OpInst to the value operand of the StoreInst to
  ///   AtomicOpnd.
  ///   (v) Final store to AtomicOpnd.
  ///
  /// \returns `true`, if \p OpInst and \p ValueOpnd are found successfully
  /// using \p AtomicOpnd as the atomic operand, `false` otherwise.
  static bool extractAtomicUpdateOp(
      BasicBlock *BB, Value *AtomicOpnd,                       // In
      Instruction *&OpInst, Value *&ValueOpnd, bool &Reversed, // Out
      StoreInst *&AtomicOpndStore,                             // Out
      SmallVectorImpl<Instruction *> &InstsToDelete);          // In, Out

  /// \brief Extracts the atomic capture op and operands for the BasicBlock \p
  /// BB. The function looks at \p BB, and identifies the atomic operand,
  /// capture operand and the value operand.
  ///
  /// Example: Consider the capture operation:
  ///    {    x = x - expr;          v = x;        }
  ///    +--update operation--+ +--copy operation--+
  ///
  ///  Here, AtomicOpnd `x` is float, ValueOpnd `expr` is double, and
  ///  CaptureOpnd `v` is uint64_t.
  ///
  /// IR:
  ///     %2 = load float, float* %x, align 4       ; <- (i)
  ///     %conv = fpext float %2 to double          ; <- (ii)
  ///     %3 = load double, double* %expr, align 8
  ///     %sub = fsub double %conv, %3              ; <- (iii)
  ///     %conv1 = fptrunc double %sub to float     ; <- (iv)
  ///     store float %conv1, float* %x, align 4    ; <- (v)
  ///     %4 = load float, float* %x, align 4       ; <- (a)
  ///     %conv2 = fptoui float %4 to i64           ; <- (b)
  ///     store i64 %conv2, i64* %v, align 8        ; <- (c)
  ///
  /// \param [in] BB is the middle BasicBlock of WRNAtomicNode.
  /// \param [out] OpInst is the LLVM Instruction for the operation which
  /// computes the result of the update operation. (%sub here)
  /// \param [out] AtomicOpnd atomic operand. (%x here)
  /// \param [out] CaptureOpnd capture operand. (%v here)
  /// \param [out] ValueOpnd value operand. (%3 here)
  /// \param [out] Reversed is `true` if in a non-commutative update operation,
  /// atomic operand `x` is the second operand. e.g.`x = expr - x`. (false here)
  /// \param [out] AtomicOpndStore is the StoreInst storing the result of \p
  /// OpInst to \p AtomicOpnd (Instruction (v) here).
  /// \param [out] CaptureOpndCast The CastInst (from the type of \p AtomicOpnd)
  /// done before the store to \p CaptureOpnd. (%conv2 here)
  /// \param [in,out] InstsToDelete is updated with the Instructions that should
  /// be deleted after inserting a call to KMPC intrinsic. (Here (i) to (v) are
  /// populated by a call to extractAtomicUpdateOp(), and (a) to (c) by
  /// identifyNonSwapCaptureKind(). For capture swap, look at extractSwapOp().).
  ///
  /// \returns the AtomicCaptureKind of the capture operation found,
  /// `CaptureUnknown` if no capture op is found.
  ///
  /// \pre There is an assumption in the current implementation regarding the
  /// position of stores to `x` and `v`:
  ///     * Store to `v` should either be the first, or last in \p BB, and
  ///     corresponding to these, store to `x` should either be the last, or
  ///     second last in \p BB.
  ///     * There should not be more than one store within \p BB to either
  ///     `v` or `x` (which is in accordance with OpenMP 4.5 spec).
  ///
  /// TODO: Receive `x` and `v` from the frontend.
  static AtomicCaptureKind extractAtomicCaptureOp(
      BasicBlock *BB,                                                   // In
      Instruction *&OpInst, Value *&AtomicOpnd, Value *&ValueOpnd,      // Out
      Value *&CaptureOpnd, bool &Reversed, StoreInst *&AtomicOpndStore, // Out
      CastInst *&CaptureOpndCast,                                       // Out
      SmallVectorImpl<Instruction *> &InstsToDelete); // In, Out

  /// \brief Returns the CaptureKind for the operation in an atomic capture
  /// construct. This function identifies non-swap kind atomic captures.
  /// \param [in] AtomicStore is the StoreInst saving the result of the atomic
  /// operation to atomic operand x. i.e. `x = x op expr'.
  /// \param [out] CaptureOpndCast The CastInst (from the type of \p AtomicOpnd
  /// to the type of \p CaptureOpnd) done before the store to \p CaptureOpnd.
  /// \param [in,out] InstsToDelete Instructions corresponding to the copy
  /// operation, that will need to be deleted if a KMPC atomic intrinsic call is
  /// inserted, are added to this list. These include:
  ///     (a) Load from AtomicOpnd, which is (cast and) stored to CaptureOpnd.
  ///     (b) CastInst of the above loaded value, being stored to CaptureOpnd.
  ///     (c) The store to CaptureOpnd.
  /// See extractAtomicCaptureOp() for examples of (a), (b) and (c).
  ///
  /// \returns the CaptureKind, which can be CaptureBeforeOp or CaptureAfterOp
  /// if the capture operation is successfully identified, CaptureUnknown
  /// otherwise.
  static AtomicCaptureKind identifyNonSwapCaptureKind(
      BasicBlock *BB, StoreInst *AtomicStore, Value *CaptureOpnd, // In
      CastInst *&CaptureOpndCast,                                 // Out
      SmallVectorImpl<Instruction *> &InstsToDelete);             // In, Out

  /// \brief Using a given \p AtomicOpnd and \p CaptureOpnd, try to find the
  /// \p ValueOpnd assuming the capture operation as swap.
  /// Example:
  ///     v = x; x = expr;
  ///     Here AtomicOpnd `x` is float, CaptureOpnd `v` is uint64_t and
  ///     ValueOpnd `expr` is double.
  /// IR:
  ///     %2 = load float, float* %x, align 4       ; <- (1)
  ///     %conv = fptoui float %2 to i64            ; <- (2)
  ///     store i64 %conv, i64* %v, align 8         ; <- (3)
  ///     %3 = load double, double* %expr, align 8
  ///     %conv1 = fptrunc double %3 to float
  ///     store float %conv1, float* %x, align 4
  ///
  /// \param [in] BB The middle BB of the WRNAtomicNode.
  /// \param [in] AtomicOpnd Atomic operand. (%x here)
  /// \param [in] CaptureOpnd capture operand. (%v)
  /// \param [out] ValueOpnd value operand. (%3 here)
  /// \param [out] AtomicOpndStore is the StoreInst which stores `expr` to `x`.
  /// (last instruction here)
  /// \param [out] CaptureOpndCast The CastInst (from the type of \p AtomicOpnd)
  /// done before the store to \p CaptureOpnd. (%conv here)
  /// \param [in,out] InstsToDelete Is appended with instructions to be deleted
  /// when a KMPC call is inserted for the atomic operation. These,
  /// corresponding to the instructions marked above, are:
  ///     (1) Load of AtomicOpnd (x) for the copy operation `v = x`.
  ///     (2) Cast of the loaded x, before store to CaptureOpnd.
  ///     (3) Store to the CaptureOpnd.
  ///
  /// \returns `true` if a swap operation with given atomic and capture operands
  /// is successfully identified, `false` otherwise.
  static bool
  extractSwapOp(BasicBlock *BB, Value *AtomicOpnd, Value *CaptureOpnd, // In
                Value *&ValueOpnd, StoreInst *&AtomicOpndStore,        // Out
                CastInst *&CaptureOpndCast,                            // Out
                SmallVectorImpl<Instruction *> &InstsToDelete); // In, Out

  /// @}

  /// \name Functions for generating a cast for the value operand for
  /// update/capture.
  /// @{

  /// \brief Generates a CastInst for \p ValueOpnd depending upon the \p Op, \p
  /// AtomicOpnd and \p AtomicKind, if needed. A cast is might be needed if:
  /// * AtomicOpnd and ValueOpnd are integer types, but AtomicOpnd has lesser
  /// number of bits.
  /// * AtomicOpnd is integer type and ValueOpnd is floating point type.
  /// * AtomicOpnd and ValueOpnd are floating point types, but AtomicOpnd has
  /// lesser number of bits.
  /// \tparam AtomicKind can either be WRNAtomicUpdate or WRNAtomicCapture.
  /// \param [in] Op is the Instruction for the update operation, and is
  /// ignored for capture operation.
  /// \param [in] Reversed is `true` if in a non-commutative update operation,
  /// atomic operand `x` is the second operand. e.g.`x = expr - x`.
  /// \param [in] AtomicOpnd Atomic operand 'x'.
  /// \param [in] ValueOpnd Value operand 'expr'.
  ///
  /// \returns The generated CastInst for \p ValueOpnd. \c nullptr if no
  /// CastInst is generated.
  template <WRNAtomicKind AtomicKind>
  static CastInst *genCastForValueOpnd(const Instruction *Op, bool Reversed,
                                       const Value *AtomicOpnd,
                                       Value *ValueOpnd);

  /// \brief Generates a Trunc cast for \p ValueOpnd in case of an update
  /// operation between integer \p AtomicOpnd and \p ValueOpnd, such that \p
  /// AtomicOpnd has lesser number of bits than \p ValueOpnd. The CastInst
  /// generated generated truncates \p ValueOpnd to the size of \p AtomicOpnd.
  /// \param [in] AtomicOpnd atomic operand.
  /// \param [in] ValueOpnd value operand.
  ///
  /// \returns the Trunc CastInst for \p ValueOpnd, if supported/needed,
  /// `nullptr` otherwise.
  static CastInst *genTruncForValueOpnd(const Value &AtomicOpnd,
                                        Value &ValueOpnd);

  /// \brief Generates FPExt cast for \p ValueOpnd in case of mixed atomic
  /// update with integer \p AtomicOpnd and floating point \p ValueOpnd, if
  /// needed. If \p AtomicOpnd is an integer, and \p ValueOpnd is a floating
  /// point type, such that an intrinsic for the combination would not exist,
  /// then a CastInst is generated to extend \p ValueOpnd to a higher-precision
  /// floating point value for which an intrinsic would exist. \p Op is needed
  /// to obtain the target size of the FPExt cast.
  /// \param [in] Op is the Instruction for the update operation.
  /// \param [in] Reversed is `true` if in a non-commutative update operation,
  /// atomic operand `x` is the second operand. e.g.`x = expr - x`.
  /// \param [in] AtomicOpnd atomic operand.
  /// \param [in] ValueOpnd value operand.
  ///
  /// \returns the FPExt CastInst for \p ValueOpnd, if supported/needed,
  /// `nullptr` otherwise.
  static CastInst *genFPExtForValueOpnd(const Instruction &Op, bool Reversed,
                                        const Value &AtomicOpnd,
                                        Value &ValueOpnd);

  /// \brief Generates FPTrunc cast for \p ValueOpnd in case of an update
  /// operation between floating point \p AtomicOpnd and \p ValueOpnd, such that
  /// \p AtomicOpnd has lesser number of bits than \p ValueOpnd. The CastInst
  /// generated truncates \p ValueOpnd to the size of \p AtomicOpnd.
  /// \tparam AtomicKind can either be WRNAtomicUpdate or WRNAtomicCapture.
  /// \param [in] AtomicOpnd atomic operand.
  /// \param [in] ValueOpnd value operand.
  ///
  /// \returns the Trunc CastInst for \p ValueOpnd, if supported/needed,
  /// `nullptr` otherwise.
  template <WRNAtomicKind AtomicKind>
  static CastInst *genFPTruncForValueOpnd(const Value &AtomicOpnd,
                                          Value& ValueOpnd);

  /// @}

  /// \name Functions for intrinsic name lookup.
  /// @{

  /// \brief Looks up the map from operand type to intrinsic name for
  /// atomic read/write and capture of kind swap.
  /// \tparam AtomicKind can be \arg WRNAtomicRead, \arg WRNAtomicWrite, or \arg
  /// WRNAtomicCapture (for swap-kind capture operation).
  /// \tparam CaptureKind is required only for \p AtomicKind = WRNAtomicCapture.
  /// And it can only be \arg CaptureSwap.
  /// \param BB is the middle BasicBlock of the WRNAtomicNode.
  /// \param OpndTy is Type of the atomic operand.
  ///
  /// \returns the intrinsic name for \p OpndTy if found, otherwise nullptr.
  template <WRNAtomicKind AtomicKind,
            AtomicCaptureKind CaptureKind = CaptureUnknown>
  static const std::string getAtomicRWSIntrinsicName(const BasicBlock &BB,
                                                     const Type &OpndTy);

  /// \brief Looks up the map from operation type to intrinsic name for atomic
  /// update and non-swap kind atomic capture. Consider the following two
  /// examples of an update operation for atomic update/capture:
  /// \code
  ///      x = x - expr;  // (1)
  ///      y = expr / y;  // (2)
  /// \endcode
  ///
  /// \tparam AtomicKind can be \arg WRNAtomicUpdate, or \arg WRNAtomicCapture
  /// (for non-swap kind capture operation).
  /// \tparam CaptureKind is required only for \p AtomicKind = WRNAtomicCapture.
  /// And it can either be \arg CaptureBeforeOp or \arg CaptureAfterOp.
  /// \param Op is the LLVM Instruction which represents the binary operation,
  /// for example, for integer operands, \p Op will be `sub` in (1) and
  /// `sdiv` in (2).
  /// \param Reversed is `true` if for a non-commutative operation like sub/div,
  /// the atomic operand is the second one. e.g. x = expr - x. (`true` for (2),
  /// `false` for (1)).
  /// \param AtomicOpnd is the operand `x` in (1), and `y` in (2).
  /// \param ValueOpnd is `expr` in (1) and (2).
  ///
  /// \returns Intrinsic name for the operation, if found, otherwise an empty
  /// string.
  template <WRNAtomicKind AtomicKind,
            AtomicCaptureKind CaptureKind = CaptureUnknown>
  static const std::string
  getAtomicUCIntrinsicName(const Instruction &Operation, bool Reversed,
                           const Value &AtomicOpnd, const Value &ValueOpnd);

  /// \brief Returns the intrinsic name for the atomic capture operation
  /// for the given operation and operands.
  /// Example:
  ///     x = x op expr; v = x; // CaptureAfterOp
  ///     v = x; x = x op expr; // CaptureBeforeOp
  ///     v = x; x = expr;      // CaptureSwap
  ///
  /// \param [in] CaptureKind is the kind of capture operation, which can be
  /// \arg CaptureBeforeOp, \arg CaptureAfterOp, or \arg CaptureSwap.
  /// \param [in] BB is the middle BasicBlock of WRNAtomicNode.
  /// \param [in] Operation (Ignored for CaptureSwap) is the Instruction
  /// corresponding to `op` for CaptureAfterOp and CaptureBeforeOp.
  /// \param [in] Reversed (Ignored for CaptureSwap) tells if for a
  /// non-commutative `op` like `sub`, atomic operand is the second operand,
  /// such as `x = expr - x`.
  /// \param [in] AtomicOpnd is `x`.
  /// \param [in] ValueOpnd is `expr`.
  ///
  /// \returns Intrinsic name for the operation, if found, otherwise an empty
  /// string.
  static const std::string getAtomicCaptureIntrinsicName(
      AtomicCaptureKind CaptureKind, const BasicBlock *BB,
      const Instruction *Operation, bool Reversed, const Value *AtomicOpnd,
      const Value *ValueOpnd);

  /// \brief Adjusts \p IntrinsicName for non-X86 architectures.
  /// The function looks for the substring '_a16' in \p IntrinsicName, removes
  /// it from the name for x64 architectures, and returns the resulting string.
  /// \param BB is the BasicBlock in which the intrinsic is to be inserted.
  ///
  /// \returns Adjusted intrinsic name if needed, original \p IntrinsicName
  /// otherwise.
  static const std::string
  adjustIntrinsicNameForArchitecture(const BasicBlock &BB,
                                     const std::string &IntrinsicName);

  /// @}

  /// \name Misc. helper methods.
  /// @{

  /// \brief Returns a SmallVector containing the first, second to last, and the
  /// last StoreInst's in \p BB, in that order.
  /// The returned SmallVector is of form:
  ///     +------------------------+
  ///     | (i) | (ii) | [ (iii) ] |
  ///     +------------------------+
  /// (i) is the first StoreInst of \p BB.
  /// (ii) is the Second-last StoreInst of \p BB. It will be the first StoreInst
  /// of \p BB if there are only two StoreInsts in BB, and the first/last
  /// StoreInst, if there is only one StoreInst in \p BB.
  /// (iii) is the last StoreInst of \p BB if there are two or more StoreInsts
  /// in BB, not present otherwise.
  static SmallVector<StoreInst *, 3>
  gatherFirstSecondToLastAndLastStores(BasicBlock &BB); // In

  /// \brief If there is only one StoreInst to \p Opnd inside BasicBlock \p
  /// BB, then return it. Otherwise, return nullptr.
  static StoreInst *getStoreToOpndIfUnique(BasicBlock &BB,     // In
                                           const Value &Opnd); // In

  /// \brief Returns `true` if Val is a UIToFP CastInst, `false` otherwise.
  static bool isUIToFPCast(const Value &Val);

  /// \brief Removes duplicate Instructions from \p Insts.
  static void
  removeDuplicateInstsFromList(SmallVectorImpl<Instruction *> &Insts);

  /// \returns `true` if any Instruction in \p Insts is used outside the
  /// BasicBlock \p BB, `false` otherwise.
  static bool
  instructionsAreUsedOutsideBB(SmallVectorImpl<Instruction *> &Insts,
                               BasicBlock *&BB);

  /// \brief Delete all Instructions in the SmallVector \p InstsToDelete.
  static void
  deleteInstructionsInList(SmallVectorImpl<Instruction *> &InstsToDelete);

  /// @}

  ///\name Constants
  /// @{
  static const unsigned ApproxNumInstsToDeleteForUpdate =
      5; ///< See extractAtomicUpdateOp() for details.
  static const unsigned ApproxNumInstsToDeleteForCapture =
      ApproxNumInstsToDeleteForUpdate +
      3; ///< See extractAtomicCaptureOp() for details.
  /// @}
};

} /// namespace vpo
} /// namespace llvm

#endif // LLVM_TRANSFORMS_VPO_PAROPT_ATOMICS_H
