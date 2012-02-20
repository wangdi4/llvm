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

declare void @__Mul2_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.Mul2_original(double addrspace(1)* nocapture, i32) nounwind

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

define void @__Mul2_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %7 = getelementptr inbounds double addrspace(1)* %data, i64 %6
  %8 = load double addrspace(1)* %7, align 8
  %9 = fsub double %8, %8
  %10 = fadd double %9, 9.990000e-01
  %11 = fsub double 1.000000e+01, %10
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %j.03 = phi i32 [ %128, %bb.nph ], [ 0, %SyncBB ]
  %s2.02 = phi double [ %127, %bb.nph ], [ %11, %SyncBB ]
  %s.01 = phi double [ %125, %bb.nph ], [ %10, %SyncBB ]
  %12 = fmul double %s.01, %s.01
  %13 = fmul double %12, 1.010000e+00
  %14 = fmul double %s2.02, %s2.02
  %15 = fmul double %14, 1.010000e+00
  %16 = fmul double %13, %13
  %17 = fmul double %16, 1.010000e+00
  %18 = fmul double %15, %15
  %19 = fmul double %18, 1.010000e+00
  %20 = fmul double %17, %17
  %21 = fmul double %20, 1.010000e+00
  %22 = fmul double %19, %19
  %23 = fmul double %22, 1.010000e+00
  %24 = fmul double %21, %21
  %25 = fmul double %24, 1.010000e+00
  %26 = fmul double %23, %23
  %27 = fmul double %26, 1.010000e+00
  %28 = fmul double %25, %25
  %29 = fmul double %28, 1.010000e+00
  %30 = fmul double %27, %27
  %31 = fmul double %30, 1.010000e+00
  %32 = fmul double %29, %29
  %33 = fmul double %32, 1.010000e+00
  %34 = fmul double %31, %31
  %35 = fmul double %34, 1.010000e+00
  %36 = fmul double %33, %33
  %37 = fmul double %36, 1.010000e+00
  %38 = fmul double %35, %35
  %39 = fmul double %38, 1.010000e+00
  %40 = fmul double %37, %37
  %41 = fmul double %40, 1.010000e+00
  %42 = fmul double %39, %39
  %43 = fmul double %42, 1.010000e+00
  %44 = fmul double %41, %41
  %45 = fmul double %44, 1.010000e+00
  %46 = fmul double %43, %43
  %47 = fmul double %46, 1.010000e+00
  %48 = fmul double %45, %45
  %49 = fmul double %48, 1.010000e+00
  %50 = fmul double %47, %47
  %51 = fmul double %50, 1.010000e+00
  %52 = fmul double %49, %49
  %53 = fmul double %52, 1.010000e+00
  %54 = fmul double %51, %51
  %55 = fmul double %54, 1.010000e+00
  %56 = fmul double %53, %53
  %57 = fmul double %56, 1.010000e+00
  %58 = fmul double %55, %55
  %59 = fmul double %58, 1.010000e+00
  %60 = fmul double %57, %57
  %61 = fmul double %60, 1.010000e+00
  %62 = fmul double %59, %59
  %63 = fmul double %62, 1.010000e+00
  %64 = fmul double %61, %61
  %65 = fmul double %64, 1.010000e+00
  %66 = fmul double %63, %63
  %67 = fmul double %66, 1.010000e+00
  %68 = fmul double %65, %65
  %69 = fmul double %68, 1.010000e+00
  %70 = fmul double %67, %67
  %71 = fmul double %70, 1.010000e+00
  %72 = fmul double %69, %69
  %73 = fmul double %72, 1.010000e+00
  %74 = fmul double %71, %71
  %75 = fmul double %74, 1.010000e+00
  %76 = fmul double %73, %73
  %77 = fmul double %76, 1.010000e+00
  %78 = fmul double %75, %75
  %79 = fmul double %78, 1.010000e+00
  %80 = fmul double %77, %77
  %81 = fmul double %80, 1.010000e+00
  %82 = fmul double %79, %79
  %83 = fmul double %82, 1.010000e+00
  %84 = fmul double %81, %81
  %85 = fmul double %84, 1.010000e+00
  %86 = fmul double %83, %83
  %87 = fmul double %86, 1.010000e+00
  %88 = fmul double %85, %85
  %89 = fmul double %88, 1.010000e+00
  %90 = fmul double %87, %87
  %91 = fmul double %90, 1.010000e+00
  %92 = fmul double %89, %89
  %93 = fmul double %92, 1.010000e+00
  %94 = fmul double %91, %91
  %95 = fmul double %94, 1.010000e+00
  %96 = fmul double %93, %93
  %97 = fmul double %96, 1.010000e+00
  %98 = fmul double %95, %95
  %99 = fmul double %98, 1.010000e+00
  %100 = fmul double %97, %97
  %101 = fmul double %100, 1.010000e+00
  %102 = fmul double %99, %99
  %103 = fmul double %102, 1.010000e+00
  %104 = fmul double %101, %101
  %105 = fmul double %104, 1.010000e+00
  %106 = fmul double %103, %103
  %107 = fmul double %106, 1.010000e+00
  %108 = fmul double %105, %105
  %109 = fmul double %108, 1.010000e+00
  %110 = fmul double %107, %107
  %111 = fmul double %110, 1.010000e+00
  %112 = fmul double %109, %109
  %113 = fmul double %112, 1.010000e+00
  %114 = fmul double %111, %111
  %115 = fmul double %114, 1.010000e+00
  %116 = fmul double %113, %113
  %117 = fmul double %116, 1.010000e+00
  %118 = fmul double %115, %115
  %119 = fmul double %118, 1.010000e+00
  %120 = fmul double %117, %117
  %121 = fmul double %120, 1.010000e+00
  %122 = fmul double %119, %119
  %123 = fmul double %122, 1.010000e+00
  %124 = fmul double %121, %121
  %125 = fmul double %124, 1.010000e+00
  %126 = fmul double %123, %123
  %127 = fmul double %126, 1.010000e+00
  %128 = add nsw i32 %j.03, 1
  %exitcond = icmp eq i32 %128, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %s2.0.lcssa = phi double [ %11, %SyncBB ], [ %127, %bb.nph ]
  %s.0.lcssa = phi double [ %10, %SyncBB ], [ %125, %bb.nph ]
  %129 = fadd double %s.0.lcssa, %s2.0.lcssa
  store double %129, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB6

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB6:                                          ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.Mul2_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB21

SyncBB21:                                         ; preds = %thenBB, %FirstBB
  %CurrWI..0 = phi i64 [ 0, %FirstBB ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %extract.lhs.lhs = shl i64 %5, 32
  %extract = ashr i64 %extract.lhs.lhs, 32
  %6 = getelementptr inbounds double addrspace(1)* %data, i64 %extract
  %ptrTypeCast = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  %7 = load <16 x double> addrspace(1)* %ptrTypeCast, align 8
  %8 = fsub <16 x double> %7, %7
  %9 = fadd <16 x double> %8, <double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01>
  %10 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %9
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB21, %bb.nph
  %j.03 = phi i32 [ %127, %bb.nph ], [ 0, %SyncBB21 ]
  %vectorPHI = phi <16 x double> [ %126, %bb.nph ], [ %10, %SyncBB21 ]
  %vectorPHI17 = phi <16 x double> [ %124, %bb.nph ], [ %9, %SyncBB21 ]
  %11 = fmul <16 x double> %vectorPHI17, %vectorPHI17
  %12 = fmul <16 x double> %11, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %13 = fmul <16 x double> %vectorPHI, %vectorPHI
  %14 = fmul <16 x double> %13, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %15 = fmul <16 x double> %12, %12
  %16 = fmul <16 x double> %15, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %17 = fmul <16 x double> %14, %14
  %18 = fmul <16 x double> %17, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %19 = fmul <16 x double> %16, %16
  %20 = fmul <16 x double> %19, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %21 = fmul <16 x double> %18, %18
  %22 = fmul <16 x double> %21, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %23 = fmul <16 x double> %20, %20
  %24 = fmul <16 x double> %23, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %25 = fmul <16 x double> %22, %22
  %26 = fmul <16 x double> %25, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %27 = fmul <16 x double> %24, %24
  %28 = fmul <16 x double> %27, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %29 = fmul <16 x double> %26, %26
  %30 = fmul <16 x double> %29, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %31 = fmul <16 x double> %28, %28
  %32 = fmul <16 x double> %31, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %33 = fmul <16 x double> %30, %30
  %34 = fmul <16 x double> %33, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %35 = fmul <16 x double> %32, %32
  %36 = fmul <16 x double> %35, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %37 = fmul <16 x double> %34, %34
  %38 = fmul <16 x double> %37, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %39 = fmul <16 x double> %36, %36
  %40 = fmul <16 x double> %39, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %41 = fmul <16 x double> %38, %38
  %42 = fmul <16 x double> %41, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %43 = fmul <16 x double> %40, %40
  %44 = fmul <16 x double> %43, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %45 = fmul <16 x double> %42, %42
  %46 = fmul <16 x double> %45, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %47 = fmul <16 x double> %44, %44
  %48 = fmul <16 x double> %47, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %49 = fmul <16 x double> %46, %46
  %50 = fmul <16 x double> %49, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %51 = fmul <16 x double> %48, %48
  %52 = fmul <16 x double> %51, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %53 = fmul <16 x double> %50, %50
  %54 = fmul <16 x double> %53, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %55 = fmul <16 x double> %52, %52
  %56 = fmul <16 x double> %55, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %57 = fmul <16 x double> %54, %54
  %58 = fmul <16 x double> %57, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %59 = fmul <16 x double> %56, %56
  %60 = fmul <16 x double> %59, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %61 = fmul <16 x double> %58, %58
  %62 = fmul <16 x double> %61, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %63 = fmul <16 x double> %60, %60
  %64 = fmul <16 x double> %63, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %65 = fmul <16 x double> %62, %62
  %66 = fmul <16 x double> %65, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %67 = fmul <16 x double> %64, %64
  %68 = fmul <16 x double> %67, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %69 = fmul <16 x double> %66, %66
  %70 = fmul <16 x double> %69, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %71 = fmul <16 x double> %68, %68
  %72 = fmul <16 x double> %71, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %73 = fmul <16 x double> %70, %70
  %74 = fmul <16 x double> %73, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %75 = fmul <16 x double> %72, %72
  %76 = fmul <16 x double> %75, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %77 = fmul <16 x double> %74, %74
  %78 = fmul <16 x double> %77, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %79 = fmul <16 x double> %76, %76
  %80 = fmul <16 x double> %79, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %81 = fmul <16 x double> %78, %78
  %82 = fmul <16 x double> %81, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %83 = fmul <16 x double> %80, %80
  %84 = fmul <16 x double> %83, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %85 = fmul <16 x double> %82, %82
  %86 = fmul <16 x double> %85, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %87 = fmul <16 x double> %84, %84
  %88 = fmul <16 x double> %87, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %89 = fmul <16 x double> %86, %86
  %90 = fmul <16 x double> %89, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %91 = fmul <16 x double> %88, %88
  %92 = fmul <16 x double> %91, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %93 = fmul <16 x double> %90, %90
  %94 = fmul <16 x double> %93, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %95 = fmul <16 x double> %92, %92
  %96 = fmul <16 x double> %95, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %97 = fmul <16 x double> %94, %94
  %98 = fmul <16 x double> %97, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %99 = fmul <16 x double> %96, %96
  %100 = fmul <16 x double> %99, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %101 = fmul <16 x double> %98, %98
  %102 = fmul <16 x double> %101, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %103 = fmul <16 x double> %100, %100
  %104 = fmul <16 x double> %103, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %105 = fmul <16 x double> %102, %102
  %106 = fmul <16 x double> %105, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %107 = fmul <16 x double> %104, %104
  %108 = fmul <16 x double> %107, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %109 = fmul <16 x double> %106, %106
  %110 = fmul <16 x double> %109, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %111 = fmul <16 x double> %108, %108
  %112 = fmul <16 x double> %111, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %113 = fmul <16 x double> %110, %110
  %114 = fmul <16 x double> %113, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %115 = fmul <16 x double> %112, %112
  %116 = fmul <16 x double> %115, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %117 = fmul <16 x double> %114, %114
  %118 = fmul <16 x double> %117, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %119 = fmul <16 x double> %116, %116
  %120 = fmul <16 x double> %119, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %121 = fmul <16 x double> %118, %118
  %122 = fmul <16 x double> %121, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %123 = fmul <16 x double> %120, %120
  %124 = fmul <16 x double> %123, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %125 = fmul <16 x double> %122, %122
  %126 = fmul <16 x double> %125, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %127 = add nsw i32 %j.03, 1
  %exitcond = icmp eq i32 %127, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB21
  %vectorPHI18 = phi <16 x double> [ %10, %SyncBB21 ], [ %126, %bb.nph ]
  %vectorPHI19 = phi <16 x double> [ %9, %SyncBB21 ], [ %124, %bb.nph ]
  %128 = fadd <16 x double> %vectorPHI19, %vectorPHI18
  %ptrTypeCast20 = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  store <16 x double> %128, <16 x double> addrspace(1)* %ptrTypeCast20, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB21

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @Mul2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
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
  %21 = getelementptr inbounds double addrspace(1)* %1, i64 %20
  %22 = load double addrspace(1)* %21, align 8
  %23 = fsub double %22, %22
  %24 = fadd double %23, 9.990000e-01
  %25 = fsub double 1.000000e+01, %24
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %j.03.i = phi i32 [ %142, %bb.nph.i ], [ 0, %SyncBB.i ]
  %s2.02.i = phi double [ %141, %bb.nph.i ], [ %25, %SyncBB.i ]
  %s.01.i = phi double [ %139, %bb.nph.i ], [ %24, %SyncBB.i ]
  %26 = fmul double %s.01.i, %s.01.i
  %27 = fmul double %26, 1.010000e+00
  %28 = fmul double %s2.02.i, %s2.02.i
  %29 = fmul double %28, 1.010000e+00
  %30 = fmul double %27, %27
  %31 = fmul double %30, 1.010000e+00
  %32 = fmul double %29, %29
  %33 = fmul double %32, 1.010000e+00
  %34 = fmul double %31, %31
  %35 = fmul double %34, 1.010000e+00
  %36 = fmul double %33, %33
  %37 = fmul double %36, 1.010000e+00
  %38 = fmul double %35, %35
  %39 = fmul double %38, 1.010000e+00
  %40 = fmul double %37, %37
  %41 = fmul double %40, 1.010000e+00
  %42 = fmul double %39, %39
  %43 = fmul double %42, 1.010000e+00
  %44 = fmul double %41, %41
  %45 = fmul double %44, 1.010000e+00
  %46 = fmul double %43, %43
  %47 = fmul double %46, 1.010000e+00
  %48 = fmul double %45, %45
  %49 = fmul double %48, 1.010000e+00
  %50 = fmul double %47, %47
  %51 = fmul double %50, 1.010000e+00
  %52 = fmul double %49, %49
  %53 = fmul double %52, 1.010000e+00
  %54 = fmul double %51, %51
  %55 = fmul double %54, 1.010000e+00
  %56 = fmul double %53, %53
  %57 = fmul double %56, 1.010000e+00
  %58 = fmul double %55, %55
  %59 = fmul double %58, 1.010000e+00
  %60 = fmul double %57, %57
  %61 = fmul double %60, 1.010000e+00
  %62 = fmul double %59, %59
  %63 = fmul double %62, 1.010000e+00
  %64 = fmul double %61, %61
  %65 = fmul double %64, 1.010000e+00
  %66 = fmul double %63, %63
  %67 = fmul double %66, 1.010000e+00
  %68 = fmul double %65, %65
  %69 = fmul double %68, 1.010000e+00
  %70 = fmul double %67, %67
  %71 = fmul double %70, 1.010000e+00
  %72 = fmul double %69, %69
  %73 = fmul double %72, 1.010000e+00
  %74 = fmul double %71, %71
  %75 = fmul double %74, 1.010000e+00
  %76 = fmul double %73, %73
  %77 = fmul double %76, 1.010000e+00
  %78 = fmul double %75, %75
  %79 = fmul double %78, 1.010000e+00
  %80 = fmul double %77, %77
  %81 = fmul double %80, 1.010000e+00
  %82 = fmul double %79, %79
  %83 = fmul double %82, 1.010000e+00
  %84 = fmul double %81, %81
  %85 = fmul double %84, 1.010000e+00
  %86 = fmul double %83, %83
  %87 = fmul double %86, 1.010000e+00
  %88 = fmul double %85, %85
  %89 = fmul double %88, 1.010000e+00
  %90 = fmul double %87, %87
  %91 = fmul double %90, 1.010000e+00
  %92 = fmul double %89, %89
  %93 = fmul double %92, 1.010000e+00
  %94 = fmul double %91, %91
  %95 = fmul double %94, 1.010000e+00
  %96 = fmul double %93, %93
  %97 = fmul double %96, 1.010000e+00
  %98 = fmul double %95, %95
  %99 = fmul double %98, 1.010000e+00
  %100 = fmul double %97, %97
  %101 = fmul double %100, 1.010000e+00
  %102 = fmul double %99, %99
  %103 = fmul double %102, 1.010000e+00
  %104 = fmul double %101, %101
  %105 = fmul double %104, 1.010000e+00
  %106 = fmul double %103, %103
  %107 = fmul double %106, 1.010000e+00
  %108 = fmul double %105, %105
  %109 = fmul double %108, 1.010000e+00
  %110 = fmul double %107, %107
  %111 = fmul double %110, 1.010000e+00
  %112 = fmul double %109, %109
  %113 = fmul double %112, 1.010000e+00
  %114 = fmul double %111, %111
  %115 = fmul double %114, 1.010000e+00
  %116 = fmul double %113, %113
  %117 = fmul double %116, 1.010000e+00
  %118 = fmul double %115, %115
  %119 = fmul double %118, 1.010000e+00
  %120 = fmul double %117, %117
  %121 = fmul double %120, 1.010000e+00
  %122 = fmul double %119, %119
  %123 = fmul double %122, 1.010000e+00
  %124 = fmul double %121, %121
  %125 = fmul double %124, 1.010000e+00
  %126 = fmul double %123, %123
  %127 = fmul double %126, 1.010000e+00
  %128 = fmul double %125, %125
  %129 = fmul double %128, 1.010000e+00
  %130 = fmul double %127, %127
  %131 = fmul double %130, 1.010000e+00
  %132 = fmul double %129, %129
  %133 = fmul double %132, 1.010000e+00
  %134 = fmul double %131, %131
  %135 = fmul double %134, 1.010000e+00
  %136 = fmul double %133, %133
  %137 = fmul double %136, 1.010000e+00
  %138 = fmul double %135, %135
  %139 = fmul double %138, 1.010000e+00
  %140 = fmul double %137, %137
  %141 = fmul double %140, 1.010000e+00
  %142 = add nsw i32 %j.03.i, 1
  %exitcond.i = icmp eq i32 %142, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %s2.0.lcssa.i = phi double [ %25, %SyncBB.i ], [ %141, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %24, %SyncBB.i ], [ %139, %bb.nph.i ]
  %143 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  store double %143, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Mul2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__Mul2_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.Mul2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
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
  br label %SyncBB21.i

SyncBB21.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %extract.lhs.lhs.i = shl i64 %19, 32
  %extract.i = ashr i64 %extract.lhs.lhs.i, 32
  %20 = getelementptr inbounds double addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  %21 = load <16 x double> addrspace(1)* %ptrTypeCast.i, align 8
  %22 = fsub <16 x double> %21, %21
  %23 = fadd <16 x double> %22, <double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01, double 9.990000e-01>
  %24 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %23
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB21.i
  %j.03.i = phi i32 [ %141, %bb.nph.i ], [ 0, %SyncBB21.i ]
  %vectorPHI.i = phi <16 x double> [ %140, %bb.nph.i ], [ %24, %SyncBB21.i ]
  %vectorPHI17.i = phi <16 x double> [ %138, %bb.nph.i ], [ %23, %SyncBB21.i ]
  %25 = fmul <16 x double> %vectorPHI17.i, %vectorPHI17.i
  %26 = fmul <16 x double> %25, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %27 = fmul <16 x double> %vectorPHI.i, %vectorPHI.i
  %28 = fmul <16 x double> %27, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %29 = fmul <16 x double> %26, %26
  %30 = fmul <16 x double> %29, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %31 = fmul <16 x double> %28, %28
  %32 = fmul <16 x double> %31, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %33 = fmul <16 x double> %30, %30
  %34 = fmul <16 x double> %33, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %35 = fmul <16 x double> %32, %32
  %36 = fmul <16 x double> %35, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %37 = fmul <16 x double> %34, %34
  %38 = fmul <16 x double> %37, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %39 = fmul <16 x double> %36, %36
  %40 = fmul <16 x double> %39, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %41 = fmul <16 x double> %38, %38
  %42 = fmul <16 x double> %41, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %43 = fmul <16 x double> %40, %40
  %44 = fmul <16 x double> %43, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %45 = fmul <16 x double> %42, %42
  %46 = fmul <16 x double> %45, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %47 = fmul <16 x double> %44, %44
  %48 = fmul <16 x double> %47, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %49 = fmul <16 x double> %46, %46
  %50 = fmul <16 x double> %49, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %51 = fmul <16 x double> %48, %48
  %52 = fmul <16 x double> %51, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %53 = fmul <16 x double> %50, %50
  %54 = fmul <16 x double> %53, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %55 = fmul <16 x double> %52, %52
  %56 = fmul <16 x double> %55, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %57 = fmul <16 x double> %54, %54
  %58 = fmul <16 x double> %57, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %59 = fmul <16 x double> %56, %56
  %60 = fmul <16 x double> %59, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %61 = fmul <16 x double> %58, %58
  %62 = fmul <16 x double> %61, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %63 = fmul <16 x double> %60, %60
  %64 = fmul <16 x double> %63, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %65 = fmul <16 x double> %62, %62
  %66 = fmul <16 x double> %65, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %67 = fmul <16 x double> %64, %64
  %68 = fmul <16 x double> %67, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %69 = fmul <16 x double> %66, %66
  %70 = fmul <16 x double> %69, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %71 = fmul <16 x double> %68, %68
  %72 = fmul <16 x double> %71, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %73 = fmul <16 x double> %70, %70
  %74 = fmul <16 x double> %73, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %75 = fmul <16 x double> %72, %72
  %76 = fmul <16 x double> %75, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %77 = fmul <16 x double> %74, %74
  %78 = fmul <16 x double> %77, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %79 = fmul <16 x double> %76, %76
  %80 = fmul <16 x double> %79, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %81 = fmul <16 x double> %78, %78
  %82 = fmul <16 x double> %81, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %83 = fmul <16 x double> %80, %80
  %84 = fmul <16 x double> %83, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %85 = fmul <16 x double> %82, %82
  %86 = fmul <16 x double> %85, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %87 = fmul <16 x double> %84, %84
  %88 = fmul <16 x double> %87, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %89 = fmul <16 x double> %86, %86
  %90 = fmul <16 x double> %89, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %91 = fmul <16 x double> %88, %88
  %92 = fmul <16 x double> %91, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %93 = fmul <16 x double> %90, %90
  %94 = fmul <16 x double> %93, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %95 = fmul <16 x double> %92, %92
  %96 = fmul <16 x double> %95, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %97 = fmul <16 x double> %94, %94
  %98 = fmul <16 x double> %97, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %99 = fmul <16 x double> %96, %96
  %100 = fmul <16 x double> %99, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %101 = fmul <16 x double> %98, %98
  %102 = fmul <16 x double> %101, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %103 = fmul <16 x double> %100, %100
  %104 = fmul <16 x double> %103, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %105 = fmul <16 x double> %102, %102
  %106 = fmul <16 x double> %105, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %107 = fmul <16 x double> %104, %104
  %108 = fmul <16 x double> %107, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %109 = fmul <16 x double> %106, %106
  %110 = fmul <16 x double> %109, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %111 = fmul <16 x double> %108, %108
  %112 = fmul <16 x double> %111, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %113 = fmul <16 x double> %110, %110
  %114 = fmul <16 x double> %113, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %115 = fmul <16 x double> %112, %112
  %116 = fmul <16 x double> %115, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %117 = fmul <16 x double> %114, %114
  %118 = fmul <16 x double> %117, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %119 = fmul <16 x double> %116, %116
  %120 = fmul <16 x double> %119, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %121 = fmul <16 x double> %118, %118
  %122 = fmul <16 x double> %121, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %123 = fmul <16 x double> %120, %120
  %124 = fmul <16 x double> %123, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %125 = fmul <16 x double> %122, %122
  %126 = fmul <16 x double> %125, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %127 = fmul <16 x double> %124, %124
  %128 = fmul <16 x double> %127, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %129 = fmul <16 x double> %126, %126
  %130 = fmul <16 x double> %129, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %131 = fmul <16 x double> %128, %128
  %132 = fmul <16 x double> %131, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %133 = fmul <16 x double> %130, %130
  %134 = fmul <16 x double> %133, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %135 = fmul <16 x double> %132, %132
  %136 = fmul <16 x double> %135, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %137 = fmul <16 x double> %134, %134
  %138 = fmul <16 x double> %137, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %139 = fmul <16 x double> %136, %136
  %140 = fmul <16 x double> %139, <double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00, double 1.010000e+00>
  %141 = add nsw i32 %j.03.i, 1
  %exitcond.i = icmp eq i32 %141, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB21.i
  %vectorPHI18.i = phi <16 x double> [ %24, %SyncBB21.i ], [ %140, %bb.nph.i ]
  %vectorPHI19.i = phi <16 x double> [ %23, %SyncBB21.i ], [ %138, %bb.nph.i ]
  %142 = fadd <16 x double> %vectorPHI19.i, %vectorPHI18.i
  %ptrTypeCast20.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  store <16 x double> %142, <16 x double> addrspace(1)* %ptrTypeCast20.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.Mul2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB21.i

____Vectorized_.Mul2_separated_args.exit:         ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Mul2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_Mul2_locals_anchor", void (i8*)* @Mul2}
!1 = metadata !{i32 0, i32 0, i32 0}
