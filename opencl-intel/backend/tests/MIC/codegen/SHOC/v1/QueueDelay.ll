; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__one_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__two_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__three_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__four_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__one_separated_args(i32 addrspace(1)* nocapture %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @__two_separated_args(i32 addrspace(1)* nocapture %b, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @__three_separated_args(i32 addrspace(1)* nocapture %c, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @__four_separated_args(i32 addrspace(1)* nocapture %d, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
  ret void
}

define void @two(i8* %pBuffer) {
entry:
  ret void
}

define void @three(i8* %pBuffer) {
entry:
  ret void
}

define void @four(i8* %pBuffer) {
entry:
  ret void
}

define void @one(i8* %pBuffer) {
entry:
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4}

!0 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__one_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *", metadata !"opencl_one_locals_anchor", void (i8*)* @one}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__two_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *", metadata !"opencl_two_locals_anchor", void (i8*)* @two}
!3 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__three_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *", metadata !"opencl_three_locals_anchor", void (i8*)* @three}
!4 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__four_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *", metadata !"opencl_four_locals_anchor", void (i8*)* @four}


