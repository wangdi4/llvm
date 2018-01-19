//=======- Intel_IntrinsicsFPGA.td - defines HLS Intrinsics -*- tablegen -*-==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
/// HLS Stream intrinsics.
/// Signature: T* (BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesValid)
def int_intel_hls_instream_read
    : Intrinsic<[ LLVMPointerType<llvm_any_ty> ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: (T*, Success) (BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesValid)
def int_intel_hls_instream_tryRead
    : Intrinsic<[ LLVMPointerType<llvm_any_ty>, llvm_i8_ty ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: T* (BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesEmpty)
def int_intel_hls_outstream_read
    : Intrinsic<[ LLVMPointerType<llvm_any_ty> ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: (T*, Success) (BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesEmpty)
def int_intel_hls_outstream_tryRead
    : Intrinsic<[ LLVMPointerType<llvm_any_ty>, llvm_i8_ty ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesValid)
def int_intel_hls_instream_write
    : Intrinsic<[],
                [
                  LLVMPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: bool (T*, BufferID, ReadyLatency, BitsPerSymbol,
///                  FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                  UsesValid)
def int_intel_hls_instream_tryWrite
    : Intrinsic<[llvm_i1_ty],
                [
                  LLVMPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, BufferID, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesEmpty)
def int_intel_hls_outstream_write
    : Intrinsic<[],
                [
                  LLVMPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: bool (T*, BufferID, ReadyLatency, BitsPerSymbol,
///                  FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                  UsesEmpty)
def int_intel_hls_outstream_tryWrite
    : Intrinsic<[llvm_i1_ty],
                [
                  LLVMPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, Size, UseSocket, DWidth, AWidth, ASpace, Latency,
///                  MaxBurst, Align, ReadWriteMode, WaitRequest)
def int_intel_hls_mm_master_init
    : Intrinsic<[],
                [
                  LLVMPointerType<llvm_any_ty>, llvm_i32_ty, llvm_i1_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,  llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, Size, UseSocket, DWidth, AWidth, ASpace, Latency,
///                  MaxBurst, Align, ReadWriteMode, WaitRequest, Index)
def int_intel_hls_mm_master_load
    : Intrinsic<[ LLVMPointerType<llvm_any_ty> ],
                [
                  LLVMPointerType<LLVMMatchType<0>>, llvm_i32_ty, llvm_i1_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
