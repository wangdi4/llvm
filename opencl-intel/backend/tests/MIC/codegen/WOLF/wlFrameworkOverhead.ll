; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__dummy_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture) nounwind readnone

declare void @____Vectorized_.dummy_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture) nounwind readnone

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  %pred = xor i1 %t, true
  ret i1 %pred
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__dummy_separated_args(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @____Vectorized_.dummy_separated_args(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, float addrspace(1)* nocapture %c, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @dummy(i8* %pBuffer) {
entry:
  ret void
}

define void @__Vectorized_.dummy(i8* %pBuffer) {
entry:
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__dummy_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *", metadata !"opencl_dummy_locals_anchor", void (i8*)* @dummy}
!1 = metadata !{i32 0, i32 0, i32 0}
