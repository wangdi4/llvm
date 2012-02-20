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

declare void @__Add2_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.Add2_original(float addrspace(1)* nocapture, i32) nounwind

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

define void @__Add2_separated_args(float addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %sext = shl i64 %5, 32
  %6 = ashr i64 %sext, 32
  %7 = getelementptr inbounds float addrspace(1)* %data, i64 %6
  %8 = load float addrspace(1)* %7, align 4
  %s2.01 = fsub float 1.000000e+01, %8
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %s2.04 = phi float [ %s2.0, %bb.nph ], [ %s2.01, %SyncBB ]
  %j.03 = phi i32 [ %44, %bb.nph ], [ 0, %SyncBB ]
  %s.02 = phi float [ %43, %bb.nph ], [ %8, %SyncBB ]
  %9 = fsub float 1.000000e+01, %s.02
  %10 = fsub float 1.000000e+01, %9
  %11 = fsub float 1.000000e+01, %10
  %12 = fsub float 1.000000e+01, %11
  %13 = fsub float 1.000000e+01, %12
  %14 = fsub float 1.000000e+01, %13
  %15 = fsub float 1.000000e+01, %14
  %16 = fsub float 1.000000e+01, %15
  %17 = fsub float 1.000000e+01, %16
  %18 = fsub float 1.000000e+01, %17
  %19 = fsub float 1.000000e+01, %18
  %20 = fsub float 1.000000e+01, %19
  %21 = fsub float 1.000000e+01, %20
  %22 = fsub float 1.000000e+01, %21
  %23 = fsub float 1.000000e+01, %22
  %24 = fsub float 1.000000e+01, %23
  %25 = fsub float 1.000000e+01, %24
  %26 = fsub float 1.000000e+01, %25
  %27 = fsub float 1.000000e+01, %26
  %28 = fsub float 1.000000e+01, %27
  %29 = fsub float 1.000000e+01, %28
  %30 = fsub float 1.000000e+01, %29
  %31 = fsub float 1.000000e+01, %30
  %32 = fsub float 1.000000e+01, %31
  %33 = fsub float 1.000000e+01, %32
  %34 = fsub float 1.000000e+01, %33
  %35 = fsub float 1.000000e+01, %34
  %36 = fsub float 1.000000e+01, %35
  %37 = fsub float 1.000000e+01, %36
  %38 = fsub float 1.000000e+01, %37
  %39 = fsub float 1.000000e+01, %38
  %40 = fsub float 1.000000e+01, %39
  %41 = fsub float 1.000000e+01, %40
  %42 = fsub float 1.000000e+01, %41
  %43 = fsub float 1.000000e+01, %42
  %44 = add nsw i32 %j.03, 1
  %45 = fsub float 1.000000e+01, %s2.04
  %46 = fsub float 1.000000e+01, %45
  %47 = fsub float 1.000000e+01, %46
  %48 = fsub float 1.000000e+01, %47
  %49 = fsub float 1.000000e+01, %48
  %50 = fsub float 1.000000e+01, %49
  %51 = fsub float 1.000000e+01, %50
  %52 = fsub float 1.000000e+01, %51
  %53 = fsub float 1.000000e+01, %52
  %54 = fsub float 1.000000e+01, %53
  %55 = fsub float 1.000000e+01, %54
  %56 = fsub float 1.000000e+01, %55
  %57 = fsub float 1.000000e+01, %56
  %58 = fsub float 1.000000e+01, %57
  %59 = fsub float 1.000000e+01, %58
  %60 = fsub float 1.000000e+01, %59
  %61 = fsub float 1.000000e+01, %60
  %62 = fsub float 1.000000e+01, %61
  %63 = fsub float 1.000000e+01, %62
  %64 = fsub float 1.000000e+01, %63
  %65 = fsub float 1.000000e+01, %64
  %66 = fsub float 1.000000e+01, %65
  %67 = fsub float 1.000000e+01, %66
  %68 = fsub float 1.000000e+01, %67
  %69 = fsub float 1.000000e+01, %68
  %70 = fsub float 1.000000e+01, %69
  %71 = fsub float 1.000000e+01, %70
  %72 = fsub float 1.000000e+01, %71
  %73 = fsub float 1.000000e+01, %72
  %74 = fsub float 1.000000e+01, %73
  %75 = fsub float 1.000000e+01, %74
  %76 = fsub float 1.000000e+01, %75
  %77 = fsub float 1.000000e+01, %76
  %78 = fsub float 1.000000e+01, %77
  %s2.0 = fsub float 1.000000e+01, %78
  %exitcond = icmp eq i32 %44, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %s2.0.lcssa = phi float [ %s2.01, %SyncBB ], [ %s2.0, %bb.nph ]
  %s.0.lcssa = phi float [ %8, %SyncBB ], [ %43, %bb.nph ]
  %79 = fadd float %s.0.lcssa, %s2.0.lcssa
  store float %79, float addrspace(1)* %7, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB7

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB7:                                          ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.Add2_separated_args(float addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %extract.lhs.lhs = shl i64 %5, 32
  %extract = ashr i64 %extract.lhs.lhs, 32
  %6 = getelementptr inbounds float addrspace(1)* %data, i64 %extract
  %ptrTypeCast = bitcast float addrspace(1)* %6 to <16 x float> addrspace(1)*
  %7 = load <16 x float> addrspace(1)* %ptrTypeCast, align 4
  %s2.0117 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %7
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %vectorPHI = phi <16 x float> [ %s2.019, %bb.nph ], [ %s2.0117, %SyncBB ]
  %j.03 = phi i32 [ %43, %bb.nph ], [ 0, %SyncBB ]
  %vectorPHI18 = phi <16 x float> [ %42, %bb.nph ], [ %7, %SyncBB ]
  %8 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %vectorPHI18
  %9 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %8
  %10 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %9
  %11 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %10
  %12 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %11
  %13 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %12
  %14 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %13
  %15 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %14
  %16 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %15
  %17 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %16
  %18 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %17
  %19 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %18
  %20 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %19
  %21 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %20
  %22 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %21
  %23 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %22
  %24 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %23
  %25 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %24
  %26 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %25
  %27 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %26
  %28 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %27
  %29 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %28
  %30 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %29
  %31 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %30
  %32 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %31
  %33 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %32
  %34 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %33
  %35 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %34
  %36 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %35
  %37 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %36
  %38 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %37
  %39 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %38
  %40 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %39
  %41 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %40
  %42 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %41
  %43 = add nsw i32 %j.03, 1
  %44 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %vectorPHI
  %45 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %44
  %46 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %45
  %47 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %46
  %48 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %47
  %49 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %48
  %50 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %49
  %51 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %50
  %52 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %51
  %53 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %52
  %54 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %53
  %55 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %54
  %56 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %55
  %57 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %56
  %58 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %57
  %59 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %58
  %60 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %59
  %61 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %60
  %62 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %61
  %63 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %62
  %64 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %63
  %65 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %64
  %66 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %65
  %67 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %66
  %68 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %67
  %69 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %68
  %70 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %69
  %71 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %70
  %72 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %71
  %73 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %72
  %74 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %73
  %75 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %74
  %76 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %75
  %77 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %76
  %s2.019 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %77
  %exitcond = icmp eq i32 %43, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %vectorPHI20 = phi <16 x float> [ %s2.0117, %SyncBB ], [ %s2.019, %bb.nph ]
  %vectorPHI21 = phi <16 x float> [ %7, %SyncBB ], [ %42, %bb.nph ]
  %78 = fadd <16 x float> %vectorPHI21, %vectorPHI20
  %ptrTypeCast22 = bitcast float addrspace(1)* %6 to <16 x float> addrspace(1)*
  store <16 x float> %78, <16 x float> addrspace(1)* %ptrTypeCast22, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB23

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB23:                                         ; preds = %._crit_edge
  ret void
}

define void @Add2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 40
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 64
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = icmp sgt i32 %4, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %sext.i = shl i64 %19, 32
  %20 = ashr i64 %sext.i, 32
  %21 = getelementptr inbounds float addrspace(1)* %1, i64 %20
  %22 = load float addrspace(1)* %21, align 4
  %s2.01.i = fsub float 1.000000e+01, %22
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %s2.04.i = phi float [ %s2.0.i, %bb.nph.i ], [ %s2.01.i, %SyncBB.i ]
  %j.03.i = phi i32 [ %58, %bb.nph.i ], [ 0, %SyncBB.i ]
  %s.02.i = phi float [ %57, %bb.nph.i ], [ %22, %SyncBB.i ]
  %23 = fsub float 1.000000e+01, %s.02.i
  %24 = fsub float 1.000000e+01, %23
  %25 = fsub float 1.000000e+01, %24
  %26 = fsub float 1.000000e+01, %25
  %27 = fsub float 1.000000e+01, %26
  %28 = fsub float 1.000000e+01, %27
  %29 = fsub float 1.000000e+01, %28
  %30 = fsub float 1.000000e+01, %29
  %31 = fsub float 1.000000e+01, %30
  %32 = fsub float 1.000000e+01, %31
  %33 = fsub float 1.000000e+01, %32
  %34 = fsub float 1.000000e+01, %33
  %35 = fsub float 1.000000e+01, %34
  %36 = fsub float 1.000000e+01, %35
  %37 = fsub float 1.000000e+01, %36
  %38 = fsub float 1.000000e+01, %37
  %39 = fsub float 1.000000e+01, %38
  %40 = fsub float 1.000000e+01, %39
  %41 = fsub float 1.000000e+01, %40
  %42 = fsub float 1.000000e+01, %41
  %43 = fsub float 1.000000e+01, %42
  %44 = fsub float 1.000000e+01, %43
  %45 = fsub float 1.000000e+01, %44
  %46 = fsub float 1.000000e+01, %45
  %47 = fsub float 1.000000e+01, %46
  %48 = fsub float 1.000000e+01, %47
  %49 = fsub float 1.000000e+01, %48
  %50 = fsub float 1.000000e+01, %49
  %51 = fsub float 1.000000e+01, %50
  %52 = fsub float 1.000000e+01, %51
  %53 = fsub float 1.000000e+01, %52
  %54 = fsub float 1.000000e+01, %53
  %55 = fsub float 1.000000e+01, %54
  %56 = fsub float 1.000000e+01, %55
  %57 = fsub float 1.000000e+01, %56
  %58 = add nsw i32 %j.03.i, 1
  %59 = fsub float 1.000000e+01, %s2.04.i
  %60 = fsub float 1.000000e+01, %59
  %61 = fsub float 1.000000e+01, %60
  %62 = fsub float 1.000000e+01, %61
  %63 = fsub float 1.000000e+01, %62
  %64 = fsub float 1.000000e+01, %63
  %65 = fsub float 1.000000e+01, %64
  %66 = fsub float 1.000000e+01, %65
  %67 = fsub float 1.000000e+01, %66
  %68 = fsub float 1.000000e+01, %67
  %69 = fsub float 1.000000e+01, %68
  %70 = fsub float 1.000000e+01, %69
  %71 = fsub float 1.000000e+01, %70
  %72 = fsub float 1.000000e+01, %71
  %73 = fsub float 1.000000e+01, %72
  %74 = fsub float 1.000000e+01, %73
  %75 = fsub float 1.000000e+01, %74
  %76 = fsub float 1.000000e+01, %75
  %77 = fsub float 1.000000e+01, %76
  %78 = fsub float 1.000000e+01, %77
  %79 = fsub float 1.000000e+01, %78
  %80 = fsub float 1.000000e+01, %79
  %81 = fsub float 1.000000e+01, %80
  %82 = fsub float 1.000000e+01, %81
  %83 = fsub float 1.000000e+01, %82
  %84 = fsub float 1.000000e+01, %83
  %85 = fsub float 1.000000e+01, %84
  %86 = fsub float 1.000000e+01, %85
  %87 = fsub float 1.000000e+01, %86
  %88 = fsub float 1.000000e+01, %87
  %89 = fsub float 1.000000e+01, %88
  %90 = fsub float 1.000000e+01, %89
  %91 = fsub float 1.000000e+01, %90
  %92 = fsub float 1.000000e+01, %91
  %s2.0.i = fsub float 1.000000e+01, %92
  %exitcond.i = icmp eq i32 %58, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %s2.0.lcssa.i = phi float [ %s2.01.i, %SyncBB.i ], [ %s2.0.i, %bb.nph.i ]
  %s.0.lcssa.i = phi float [ %22, %SyncBB.i ], [ %57, %bb.nph.i ]
  %93 = fadd float %s.0.lcssa.i, %s2.0.lcssa.i
  store float %93, float addrspace(1)* %21, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Add2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__Add2_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.Add2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 40
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 64
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = icmp sgt i32 %4, 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %extract.lhs.lhs.i = shl i64 %19, 32
  %extract.i = ashr i64 %extract.lhs.lhs.i, 32
  %20 = getelementptr inbounds float addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast float addrspace(1)* %20 to <16 x float> addrspace(1)*
  %21 = load <16 x float> addrspace(1)* %ptrTypeCast.i, align 4
  %s2.0117.i = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %21
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %vectorPHI.i = phi <16 x float> [ %s2.019.i, %bb.nph.i ], [ %s2.0117.i, %SyncBB.i ]
  %j.03.i = phi i32 [ %57, %bb.nph.i ], [ 0, %SyncBB.i ]
  %vectorPHI18.i = phi <16 x float> [ %56, %bb.nph.i ], [ %21, %SyncBB.i ]
  %22 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %vectorPHI18.i
  %23 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %22
  %24 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %23
  %25 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %24
  %26 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %25
  %27 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %26
  %28 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %27
  %29 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %28
  %30 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %29
  %31 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %30
  %32 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %31
  %33 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %32
  %34 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %33
  %35 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %34
  %36 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %35
  %37 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %36
  %38 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %37
  %39 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %38
  %40 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %39
  %41 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %40
  %42 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %41
  %43 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %42
  %44 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %43
  %45 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %44
  %46 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %45
  %47 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %46
  %48 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %47
  %49 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %48
  %50 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %49
  %51 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %50
  %52 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %51
  %53 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %52
  %54 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %53
  %55 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %54
  %56 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %55
  %57 = add nsw i32 %j.03.i, 1
  %58 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %vectorPHI.i
  %59 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %58
  %60 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %59
  %61 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %60
  %62 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %61
  %63 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %62
  %64 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %63
  %65 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %64
  %66 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %65
  %67 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %66
  %68 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %67
  %69 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %68
  %70 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %69
  %71 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %70
  %72 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %71
  %73 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %72
  %74 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %73
  %75 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %74
  %76 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %75
  %77 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %76
  %78 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %77
  %79 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %78
  %80 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %79
  %81 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %80
  %82 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %81
  %83 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %82
  %84 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %83
  %85 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %84
  %86 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %85
  %87 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %86
  %88 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %87
  %89 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %88
  %90 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %89
  %91 = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %90
  %s2.019.i = fsub <16 x float> <float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01, float 1.000000e+01>, %91
  %exitcond.i = icmp eq i32 %57, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %vectorPHI20.i = phi <16 x float> [ %s2.0117.i, %SyncBB.i ], [ %s2.019.i, %bb.nph.i ]
  %vectorPHI21.i = phi <16 x float> [ %21, %SyncBB.i ], [ %56, %bb.nph.i ]
  %92 = fadd <16 x float> %vectorPHI21.i, %vectorPHI20.i
  %ptrTypeCast22.i = bitcast float addrspace(1)* %20 to <16 x float> addrspace(1)*
  store <16 x float> %92, <16 x float> addrspace(1)* %ptrTypeCast22.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.Add2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.Add2_separated_args.exit:         ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Add2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_Add2_locals_anchor", void (i8*)* @Add2}
!1 = metadata !{i32 0, i32 0, i32 0}
