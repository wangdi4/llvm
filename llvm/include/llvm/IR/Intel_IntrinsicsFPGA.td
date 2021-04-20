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
/// Signature: (T*, SOP, EOP, Empty) (BufferID, Buffer, ReadyLatency,
///               BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets,
///               UsesEmpty, UsesValid)
def int_intel_hls_instream_read
    : Intrinsic<
          [ LLVMAnyPointerType<llvm_any_ty>, llvm_i8_ty, llvm_i8_ty, llvm_i32_ty ],
          [
            llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty,
            llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
          ],
          []>;
/// Signature: (T*, Success, SOP, EOP, Empty) (BufferID, Buffer, ReadyLatency,
///               BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets,
///               UsesEmpty, UsesValid)
def int_intel_hls_instream_tryRead
    : Intrinsic<[
      LLVMAnyPointerType<llvm_any_ty>, llvm_i8_ty, llvm_i8_ty, llvm_i8_ty,
      llvm_i32_ty
    ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: (T*, SOP, EOP, Empty) (BufferID, Buffer, ReadyLatency,
///               BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets,
///               UsesEmpty, UsesValid)
def int_intel_hls_outstream_read
    : Intrinsic<
          [ LLVMAnyPointerType<llvm_any_ty>, llvm_i8_ty, llvm_i8_ty, llvm_i32_ty ],
          [
            llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty,
            llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
          ],
          []>;
/// Signature: (T*, Success, SOP, EOP, Empty) (BufferID, Buffer, ReadyLatency,
///               BitsPerSymbol, FirstSymbolInHighOrderBits, UsesPackets,
///               UsesEmpty, UsesValid)
def int_intel_hls_outstream_tryRead
    : Intrinsic<[
      LLVMAnyPointerType<llvm_any_ty>, llvm_i8_ty, llvm_i8_ty, llvm_i8_ty,
      llvm_i32_ty
    ],
                [
                  llvm_i64_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, BufferID, Buffer, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesValid, SOP, EOP, Empty)
def int_intel_hls_instream_write
    : Intrinsic<[],
                [
                  LLVMAnyPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
/// Signature: bool (T*, BufferID, Buffer, ReadyLatency, BitsPerSymbol,
///                  FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                  UsesValid, SOP, EOP, Empty)
def int_intel_hls_instream_tryWrite
    : Intrinsic<[ llvm_i1_ty ],
                [
                  LLVMAnyPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
/// Signature: void (T*, BufferID, Buffer, ReadyLatency, BitsPerSymbol,
///                FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                UsesValid, SOP, EOP, Empty)
def int_intel_hls_outstream_write
    : Intrinsic<[],
                [
                  LLVMAnyPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
/// Signature: bool (T*, BufferID, Buffer, ReadyLatency, BitsPerSymbol,
///                  FirstSymbolInHighOrderBits, UsesPackets, UsesEmpty,
///                  UsesValid, SOP, EOP, Empty)
def int_intel_hls_outstream_tryWrite
    : Intrinsic<[ llvm_i1_ty ],
                [
                  LLVMAnyPointerType<llvm_any_ty>, llvm_i64_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i1_ty, llvm_i1_ty,
                  llvm_i1_ty, llvm_i1_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
/// Signature: void (T*, Size, UseSocket, DWidth, AWidth, ASpace, Latency,
///                  MaxBurst, Align, ReadWriteMode, WaitRequest)
def int_intel_hls_mm_host_init
    : Intrinsic<[],
                [
                  LLVMAnyPointerType<llvm_any_ty>, llvm_i32_ty, llvm_i1_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,  llvm_i1_ty
                ],
                []>;
/// Signature: void (T*, Size, UseSocket, DWidth, AWidth, ASpace, Latency,
///                  MaxBurst, Align, ReadWriteMode, WaitRequest, Index)
def int_intel_hls_mm_host_load
    : Intrinsic<[ LLVMAnyPointerType<llvm_any_ty> ],
                [
                  LLVMMatchType<0>, llvm_i32_ty, llvm_i1_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty,
                  llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i1_ty, llvm_i32_ty
                ],
                []>;
/// Signature: T (T)
def int_fpga_reg
    : Intrinsic<
          [ llvm_any_ty ],
          [ LLVMMatchType<0> ],
          []>;
/// Signature: void (T*, T*)
def int_fpga_reg_struct
    : Intrinsic<
          [],
          [ llvm_any_ty, LLVMMatchType<0> ],
          []>;
