; RUN: opt < %s -mtriple=x86_64-pc-linux -mattr=+avx -interleaved-access -S | FileCheck %s
; RUN: opt < %s -mtriple=x86_64-pc-linux -mattr=+avx2 -interleaved-access -S | FileCheck %s
; RUN: opt < %s -mtriple=x86_64-pc-linux -mattr=+avx512f -mattr=+avx512bw -mattr=+avx512vl -interleaved-access -S | FileCheck %s

; This test file is derived from interleaved-access_Intel.ll, but uses
; non-default address spaces. These tests used to crash the compiler.
; CMPLRLLVM-22304

define <8 x i32> @interleaved_load_vf8_i32_stride4(ptr addrspace(1) %ptr){
; CHECK-LABEL: @interleaved_load_vf8_i32_stride4(
; CHECK-NOT:    bitcast ptr addrspace(1) %ptr to ptr addrspace(1)
  %wide.vec = load <32 x i32>, ptr addrspace(1) %ptr, align 16
  %v1 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef, <8 x i32> <i32 0, i32 4,  i32 8,  i32 12, i32 16, i32 20, i32 24, i32 28>
  %v2 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef, <8 x i32> <i32 1, i32 5,  i32 9,  i32 13, i32 17, i32 21, i32 25, i32 29>
  %v3 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef, <8 x i32> <i32 2, i32 6,  i32 10, i32 14, i32 18, i32 22, i32 26, i32 30>
  %v4 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef, <8 x i32> <i32 3, i32 7, i32 11, i32 15, i32 19, i32 23, i32 27, i32 31>
  %add1 = add <8 x i32> %v1, %v2
  %add2 = add <8 x i32> %v3, %v4
  %add3 = add <8 x i32> %add1, %add2
  ret <8 x i32> %add3
}

define void @interleaved_store_vf8_i32_stride4(<8 x i32> %a0,<8 x i32> %b0,<8 x i32> %c0,<8 x i32> %d0,ptr addrspace(1) %ptr){
; CHECK-LABEL: @interleaved_store_vf8_i32_stride4(
; CHECK-NOT:        bitcast ptr addrspace(1) %ptr to ptr addrspace(1)
  %store.data0 = shufflevector <8 x i32> %a0, <8 x i32> %b0, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %store.data1 = shufflevector <8 x i32> %c0, <8 x i32> %d0, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %interleave.vec = shufflevector <16 x i32> %store.data0, <16 x i32> %store.data1, <32 x i32> <i32 0, i32 8, i32 16, i32 24, i32 1, i32 9, i32 17, i32 25, i32 2, i32 10, i32 18, i32 26, i32 3, i32 11, i32 19, i32 27, i32 4, i32 12, i32 20, i32 28, i32 5, i32 13, i32 21, i32 29, i32 6, i32 14, i32 22, i32 30, i32 7, i32 15, i32 23, i32 31>
  store <32 x i32> %interleave.vec, ptr addrspace(1) %ptr, align 16
  ret void
}
