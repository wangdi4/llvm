; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc
;
;
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__Triad_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, float) nounwind

declare i64 @get_global_id(i32)

declare void @____Vectorized_.Triad_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, float) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__Triad_separated_args(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* nocapture %memC, float %s, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  br label %SyncBB

SyncBB:                                           ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %sext = shl i64 %5, 32
  %6 = ashr i64 %sext, 32
  %7 = getelementptr inbounds float addrspace(1)* %memA, i64 %6
  %8 = load float addrspace(1)* %7, align 4
  %9 = getelementptr inbounds float addrspace(1)* %memB, i64 %6
  %10 = load float addrspace(1)* %9, align 4
  %11 = fmul float %10, %s
  %12 = fadd float %8, %11
  %13 = getelementptr inbounds float addrspace(1)* %memC, i64 %6
  store float %12, float addrspace(1)* %13, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %SyncBB
  ret void
}

define void @____Vectorized_.Triad_separated_args(float addrspace(1)* nocapture %memA, float addrspace(1)* nocapture %memB, float addrspace(1)* nocapture %memC, float %s, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %temp = insertelement <16 x float> undef, float %s, i32 0
  %vector = shufflevector <16 x float> %temp, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB19

SyncBB19:                                         ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %extract.lhs.lhs = shl i64 %4, 32
  %extract = ashr i64 %extract.lhs.lhs, 32
  %5 = getelementptr inbounds float addrspace(1)* %memA, i64 %extract
  %ptrTypeCast = bitcast float addrspace(1)* %5 to <16 x float> addrspace(1)*
  %6 = load <16 x float> addrspace(1)* %ptrTypeCast, align 4
  %7 = getelementptr inbounds float addrspace(1)* %memB, i64 %extract
  %ptrTypeCast17 = bitcast float addrspace(1)* %7 to <16 x float> addrspace(1)*
  %8 = load <16 x float> addrspace(1)* %ptrTypeCast17, align 4
  %9 = fmul <16 x float> %8, %vector
  %10 = fadd <16 x float> %6, %9
  %11 = getelementptr inbounds float addrspace(1)* %memC, i64 %extract
  %ptrTypeCast18 = bitcast float addrspace(1)* %11 to <16 x float> addrspace(1)*
  store <16 x float> %10, <16 x float> addrspace(1)* %ptrTypeCast18, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB19
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB19

SyncBB:                                           ; preds = %SyncBB19
  ret void
}

define void @Triad(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float*
  %10 = load float* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %20 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = add i64 %21, %23
  %sext.i = shl i64 %24, 32
  %25 = ashr i64 %sext.i, 32
  %26 = getelementptr inbounds float addrspace(1)* %1, i64 %25
  %27 = load float addrspace(1)* %26, align 4
  %28 = getelementptr inbounds float addrspace(1)* %4, i64 %25
  %29 = load float addrspace(1)* %28, align 4
  %30 = fmul float %29, %10
  %31 = fadd float %27, %30
  %32 = getelementptr inbounds float addrspace(1)* %7, i64 %25
  store float %31, float addrspace(1)* %32, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Triad_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__Triad_separated_args.exit:                      ; preds = %SyncBB.i
  ret void
}

define void @__Vectorized_.Triad(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float*
  %10 = load float* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 80
  %18 = bitcast i8* %17 to i64*
  %19 = load i64* %18, align 8
  %temp.i = insertelement <16 x float> undef, float %10, i32 0
  %vector.i = shufflevector <16 x float> %temp.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %SyncBB19.i

SyncBB19.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %20 = getelementptr %struct.PaddedDimId* %16, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %13, i64 0, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = add i64 %21, %23
  %extract.lhs.lhs.i = shl i64 %24, 32
  %extract.i = ashr i64 %extract.lhs.lhs.i, 32
  %25 = getelementptr inbounds float addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast float addrspace(1)* %25 to <16 x float> addrspace(1)*
  %26 = load <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %27 = getelementptr inbounds float addrspace(1)* %4, i64 %extract.i
  %ptrTypeCast17.i = bitcast float addrspace(1)* %27 to <16 x float> addrspace(1)*
  %28 = load <16 x float> addrspace(1)* %ptrTypeCast17.i, align 4
  %29 = fmul <16 x float> %28, %vector.i
  %30 = fadd <16 x float> %26, %29
  %31 = getelementptr inbounds float addrspace(1)* %7, i64 %extract.i
  %ptrTypeCast18.i = bitcast float addrspace(1)* %31 to <16 x float> addrspace(1)*
  store <16 x float> %30, <16 x float> addrspace(1)* %ptrTypeCast18.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %19
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.Triad_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB19.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB19.i

____Vectorized_.Triad_separated_args.exit:        ; preds = %SyncBB19.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Triad_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float const", metadata !"opencl_Triad_locals_anchor", void (i8*)* @Triad}
!1 = metadata !{i32 0, i32 0, i32 0}
