; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__kernel1_original(i32) nounwind readnone

declare void @__kernel2_original(i32) nounwind readnone

declare void @__kernel3_original(i32) nounwind readnone

declare void @__kernel4_original(i32) nounwind readnone

declare void @__kernel5_original(i32) nounwind readnone

declare void @__kernel6_original(i32) nounwind readnone

declare void @__kernel7_original(i32) nounwind readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__kernel1_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel2_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel3_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel4_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel5_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel6_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @__kernel7_separated_args(i32 %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone alwaysinline {
entry:
  ret void
}

define void @kernel1(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel6(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel2(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel4(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel7(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel5(i8* %pBuffer) {
entry:
  ret void
}

define void @kernel3(i8* %pBuffer) {
entry:
  ret void
}

!opencl.kernels = !{!0, !6, !7, !8, !9, !10, !11}
!opencl.build.options = !{}

!0 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_kernel1_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @kernel1}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"a"}
!6 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel2_separated_args, metadata !1, metadata !1, metadata !"int", metadata !"int", metadata !"opencl_kernel2_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(int)))", void (i8*)* @kernel2}
!7 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel3_separated_args, metadata !1, metadata !1, metadata !"float", metadata !"int", metadata !"opencl_kernel3_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(float)))", void (i8*)* @kernel3}
!8 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel4_separated_args, metadata !1, metadata !1, metadata !"short", metadata !"int", metadata !"opencl_kernel4_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(short)))", void (i8*)* @kernel4}
!9 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel5_separated_args, metadata !1, metadata !1, metadata !"int4", metadata !"int", metadata !"opencl_kernel5_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(int4)))", void (i8*)* @kernel5}
!10 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel6_separated_args, metadata !1, metadata !1, metadata !"float4", metadata !"int", metadata !"opencl_kernel6_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(float4)))", void (i8*)* @kernel6}
!11 = metadata !{void (i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__kernel7_separated_args, metadata !1, metadata !1, metadata !"short16", metadata !"int", metadata !"opencl_kernel7_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"__attribute__((vec_type_hint(short16)))", void (i8*)* @kernel7}
