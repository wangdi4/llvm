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

declare void @__MulMAdd1_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.MulMAdd1_original(double addrspace(1)* nocapture, i32) nounwind

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

define void @__MulMAdd1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB, %FirstBB
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
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB3, %bb.nph
  %j.02 = phi i32 [ %147, %bb.nph ], [ 0, %SyncBB3 ]
  %s.01 = phi double [ %146, %bb.nph ], [ %8, %SyncBB3 ]
  %9 = fmul double %s.01, 3.550000e-01
  %10 = fsub double 3.750000e+00, %9
  %11 = fmul double %10, %s.01
  %12 = fmul double %11, 3.550000e-01
  %13 = fsub double 3.750000e+00, %12
  %14 = fmul double %13, %11
  %15 = fmul double %14, 3.550000e-01
  %16 = fsub double 3.750000e+00, %15
  %17 = fmul double %16, %14
  %18 = fmul double %17, 3.550000e-01
  %19 = fsub double 3.750000e+00, %18
  %20 = fmul double %19, %17
  %21 = fmul double %20, 3.550000e-01
  %22 = fsub double 3.750000e+00, %21
  %23 = fmul double %22, %20
  %24 = fmul double %23, 3.550000e-01
  %25 = fsub double 3.750000e+00, %24
  %26 = fmul double %25, %23
  %27 = fmul double %26, 3.550000e-01
  %28 = fsub double 3.750000e+00, %27
  %29 = fmul double %28, %26
  %30 = fmul double %29, 3.550000e-01
  %31 = fsub double 3.750000e+00, %30
  %32 = fmul double %31, %29
  %33 = fmul double %32, 3.550000e-01
  %34 = fsub double 3.750000e+00, %33
  %35 = fmul double %34, %32
  %36 = fmul double %35, 3.550000e-01
  %37 = fsub double 3.750000e+00, %36
  %38 = fmul double %37, %35
  %39 = fmul double %38, 3.550000e-01
  %40 = fsub double 3.750000e+00, %39
  %41 = fmul double %40, %38
  %42 = fmul double %41, 3.550000e-01
  %43 = fsub double 3.750000e+00, %42
  %44 = fmul double %43, %41
  %45 = fmul double %44, 3.550000e-01
  %46 = fsub double 3.750000e+00, %45
  %47 = fmul double %46, %44
  %48 = fmul double %47, 3.550000e-01
  %49 = fsub double 3.750000e+00, %48
  %50 = fmul double %49, %47
  %51 = fmul double %50, 3.550000e-01
  %52 = fsub double 3.750000e+00, %51
  %53 = fmul double %52, %50
  %54 = fmul double %53, 3.550000e-01
  %55 = fsub double 3.750000e+00, %54
  %56 = fmul double %55, %53
  %57 = fmul double %56, 3.550000e-01
  %58 = fsub double 3.750000e+00, %57
  %59 = fmul double %58, %56
  %60 = fmul double %59, 3.550000e-01
  %61 = fsub double 3.750000e+00, %60
  %62 = fmul double %61, %59
  %63 = fmul double %62, 3.550000e-01
  %64 = fsub double 3.750000e+00, %63
  %65 = fmul double %64, %62
  %66 = fmul double %65, 3.550000e-01
  %67 = fsub double 3.750000e+00, %66
  %68 = fmul double %67, %65
  %69 = fmul double %68, 3.550000e-01
  %70 = fsub double 3.750000e+00, %69
  %71 = fmul double %70, %68
  %72 = fmul double %71, 3.550000e-01
  %73 = fsub double 3.750000e+00, %72
  %74 = fmul double %73, %71
  %75 = fmul double %74, 3.550000e-01
  %76 = fsub double 3.750000e+00, %75
  %77 = fmul double %76, %74
  %78 = fmul double %77, 3.550000e-01
  %79 = fsub double 3.750000e+00, %78
  %80 = fmul double %79, %77
  %81 = fmul double %80, 3.550000e-01
  %82 = fsub double 3.750000e+00, %81
  %83 = fmul double %82, %80
  %84 = fmul double %83, 3.550000e-01
  %85 = fsub double 3.750000e+00, %84
  %86 = fmul double %85, %83
  %87 = fmul double %86, 3.550000e-01
  %88 = fsub double 3.750000e+00, %87
  %89 = fmul double %88, %86
  %90 = fmul double %89, 3.550000e-01
  %91 = fsub double 3.750000e+00, %90
  %92 = fmul double %91, %89
  %93 = fmul double %92, 3.550000e-01
  %94 = fsub double 3.750000e+00, %93
  %95 = fmul double %94, %92
  %96 = fmul double %95, 3.550000e-01
  %97 = fsub double 3.750000e+00, %96
  %98 = fmul double %97, %95
  %99 = fmul double %98, 3.550000e-01
  %100 = fsub double 3.750000e+00, %99
  %101 = fmul double %100, %98
  %102 = fmul double %101, 3.550000e-01
  %103 = fsub double 3.750000e+00, %102
  %104 = fmul double %103, %101
  %105 = fmul double %104, 3.550000e-01
  %106 = fsub double 3.750000e+00, %105
  %107 = fmul double %106, %104
  %108 = fmul double %107, 3.550000e-01
  %109 = fsub double 3.750000e+00, %108
  %110 = fmul double %109, %107
  %111 = fmul double %110, 3.550000e-01
  %112 = fsub double 3.750000e+00, %111
  %113 = fmul double %112, %110
  %114 = fmul double %113, 3.550000e-01
  %115 = fsub double 3.750000e+00, %114
  %116 = fmul double %115, %113
  %117 = fmul double %116, 3.550000e-01
  %118 = fsub double 3.750000e+00, %117
  %119 = fmul double %118, %116
  %120 = fmul double %119, 3.550000e-01
  %121 = fsub double 3.750000e+00, %120
  %122 = fmul double %121, %119
  %123 = fmul double %122, 3.550000e-01
  %124 = fsub double 3.750000e+00, %123
  %125 = fmul double %124, %122
  %126 = fmul double %125, 3.550000e-01
  %127 = fsub double 3.750000e+00, %126
  %128 = fmul double %127, %125
  %129 = fmul double %128, 3.550000e-01
  %130 = fsub double 3.750000e+00, %129
  %131 = fmul double %130, %128
  %132 = fmul double %131, 3.550000e-01
  %133 = fsub double 3.750000e+00, %132
  %134 = fmul double %133, %131
  %135 = fmul double %134, 3.550000e-01
  %136 = fsub double 3.750000e+00, %135
  %137 = fmul double %136, %134
  %138 = fmul double %137, 3.550000e-01
  %139 = fsub double 3.750000e+00, %138
  %140 = fmul double %139, %137
  %141 = fmul double %140, 3.550000e-01
  %142 = fsub double 3.750000e+00, %141
  %143 = fmul double %142, %140
  %144 = fmul double %143, 3.550000e-01
  %145 = fsub double 3.750000e+00, %144
  %146 = fmul double %145, %143
  %147 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %147, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB3
  %s.0.lcssa = phi double [ %8, %SyncBB3 ], [ %146, %bb.nph ]
  store double %s.0.lcssa, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.MulMAdd1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB19

SyncBB19:                                         ; preds = %thenBB, %FirstBB
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
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB19, %bb.nph
  %j.02 = phi i32 [ %146, %bb.nph ], [ 0, %SyncBB19 ]
  %vectorPHI = phi <16 x double> [ %145, %bb.nph ], [ %7, %SyncBB19 ]
  %8 = fmul <16 x double> %vectorPHI, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %9 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %8
  %10 = fmul <16 x double> %9, %vectorPHI
  %11 = fmul <16 x double> %10, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %12 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %11
  %13 = fmul <16 x double> %12, %10
  %14 = fmul <16 x double> %13, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %15 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %14
  %16 = fmul <16 x double> %15, %13
  %17 = fmul <16 x double> %16, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %18 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %17
  %19 = fmul <16 x double> %18, %16
  %20 = fmul <16 x double> %19, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %21 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %20
  %22 = fmul <16 x double> %21, %19
  %23 = fmul <16 x double> %22, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %24 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %23
  %25 = fmul <16 x double> %24, %22
  %26 = fmul <16 x double> %25, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %27 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %26
  %28 = fmul <16 x double> %27, %25
  %29 = fmul <16 x double> %28, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %30 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %29
  %31 = fmul <16 x double> %30, %28
  %32 = fmul <16 x double> %31, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %33 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %32
  %34 = fmul <16 x double> %33, %31
  %35 = fmul <16 x double> %34, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %36 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %35
  %37 = fmul <16 x double> %36, %34
  %38 = fmul <16 x double> %37, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %39 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %38
  %40 = fmul <16 x double> %39, %37
  %41 = fmul <16 x double> %40, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %42 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %41
  %43 = fmul <16 x double> %42, %40
  %44 = fmul <16 x double> %43, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %45 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %44
  %46 = fmul <16 x double> %45, %43
  %47 = fmul <16 x double> %46, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %48 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %47
  %49 = fmul <16 x double> %48, %46
  %50 = fmul <16 x double> %49, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %51 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %50
  %52 = fmul <16 x double> %51, %49
  %53 = fmul <16 x double> %52, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %54 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %53
  %55 = fmul <16 x double> %54, %52
  %56 = fmul <16 x double> %55, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %57 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %56
  %58 = fmul <16 x double> %57, %55
  %59 = fmul <16 x double> %58, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %60 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %59
  %61 = fmul <16 x double> %60, %58
  %62 = fmul <16 x double> %61, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %63 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %62
  %64 = fmul <16 x double> %63, %61
  %65 = fmul <16 x double> %64, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %66 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %65
  %67 = fmul <16 x double> %66, %64
  %68 = fmul <16 x double> %67, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %69 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %68
  %70 = fmul <16 x double> %69, %67
  %71 = fmul <16 x double> %70, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %72 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %71
  %73 = fmul <16 x double> %72, %70
  %74 = fmul <16 x double> %73, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %75 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %74
  %76 = fmul <16 x double> %75, %73
  %77 = fmul <16 x double> %76, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %78 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %77
  %79 = fmul <16 x double> %78, %76
  %80 = fmul <16 x double> %79, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %81 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %80
  %82 = fmul <16 x double> %81, %79
  %83 = fmul <16 x double> %82, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %84 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %83
  %85 = fmul <16 x double> %84, %82
  %86 = fmul <16 x double> %85, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %87 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %86
  %88 = fmul <16 x double> %87, %85
  %89 = fmul <16 x double> %88, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %90 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %89
  %91 = fmul <16 x double> %90, %88
  %92 = fmul <16 x double> %91, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %93 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %92
  %94 = fmul <16 x double> %93, %91
  %95 = fmul <16 x double> %94, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %96 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %95
  %97 = fmul <16 x double> %96, %94
  %98 = fmul <16 x double> %97, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %99 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %98
  %100 = fmul <16 x double> %99, %97
  %101 = fmul <16 x double> %100, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %102 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %101
  %103 = fmul <16 x double> %102, %100
  %104 = fmul <16 x double> %103, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %105 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %104
  %106 = fmul <16 x double> %105, %103
  %107 = fmul <16 x double> %106, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %108 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %107
  %109 = fmul <16 x double> %108, %106
  %110 = fmul <16 x double> %109, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %111 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %110
  %112 = fmul <16 x double> %111, %109
  %113 = fmul <16 x double> %112, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %114 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %113
  %115 = fmul <16 x double> %114, %112
  %116 = fmul <16 x double> %115, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %117 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %116
  %118 = fmul <16 x double> %117, %115
  %119 = fmul <16 x double> %118, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %120 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %119
  %121 = fmul <16 x double> %120, %118
  %122 = fmul <16 x double> %121, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %123 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %122
  %124 = fmul <16 x double> %123, %121
  %125 = fmul <16 x double> %124, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %126 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %125
  %127 = fmul <16 x double> %126, %124
  %128 = fmul <16 x double> %127, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %129 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %128
  %130 = fmul <16 x double> %129, %127
  %131 = fmul <16 x double> %130, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %132 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %131
  %133 = fmul <16 x double> %132, %130
  %134 = fmul <16 x double> %133, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %135 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %134
  %136 = fmul <16 x double> %135, %133
  %137 = fmul <16 x double> %136, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %138 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %137
  %139 = fmul <16 x double> %138, %136
  %140 = fmul <16 x double> %139, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %141 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %140
  %142 = fmul <16 x double> %141, %139
  %143 = fmul <16 x double> %142, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %144 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %143
  %145 = fmul <16 x double> %144, %142
  %146 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %146, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB19
  %vectorPHI17 = phi <16 x double> [ %7, %SyncBB19 ], [ %145, %bb.nph ]
  %ptrTypeCast18 = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  store <16 x double> %vectorPHI17, <16 x double> addrspace(1)* %ptrTypeCast18, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB19

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @MulMAdd1(i8* %pBuffer) {
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
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB.i, %entry
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
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB3.i
  %j.02.i = phi i32 [ %161, %bb.nph.i ], [ 0, %SyncBB3.i ]
  %s.01.i = phi double [ %160, %bb.nph.i ], [ %22, %SyncBB3.i ]
  %23 = fmul double %s.01.i, 3.550000e-01
  %24 = fsub double 3.750000e+00, %23
  %25 = fmul double %24, %s.01.i
  %26 = fmul double %25, 3.550000e-01
  %27 = fsub double 3.750000e+00, %26
  %28 = fmul double %27, %25
  %29 = fmul double %28, 3.550000e-01
  %30 = fsub double 3.750000e+00, %29
  %31 = fmul double %30, %28
  %32 = fmul double %31, 3.550000e-01
  %33 = fsub double 3.750000e+00, %32
  %34 = fmul double %33, %31
  %35 = fmul double %34, 3.550000e-01
  %36 = fsub double 3.750000e+00, %35
  %37 = fmul double %36, %34
  %38 = fmul double %37, 3.550000e-01
  %39 = fsub double 3.750000e+00, %38
  %40 = fmul double %39, %37
  %41 = fmul double %40, 3.550000e-01
  %42 = fsub double 3.750000e+00, %41
  %43 = fmul double %42, %40
  %44 = fmul double %43, 3.550000e-01
  %45 = fsub double 3.750000e+00, %44
  %46 = fmul double %45, %43
  %47 = fmul double %46, 3.550000e-01
  %48 = fsub double 3.750000e+00, %47
  %49 = fmul double %48, %46
  %50 = fmul double %49, 3.550000e-01
  %51 = fsub double 3.750000e+00, %50
  %52 = fmul double %51, %49
  %53 = fmul double %52, 3.550000e-01
  %54 = fsub double 3.750000e+00, %53
  %55 = fmul double %54, %52
  %56 = fmul double %55, 3.550000e-01
  %57 = fsub double 3.750000e+00, %56
  %58 = fmul double %57, %55
  %59 = fmul double %58, 3.550000e-01
  %60 = fsub double 3.750000e+00, %59
  %61 = fmul double %60, %58
  %62 = fmul double %61, 3.550000e-01
  %63 = fsub double 3.750000e+00, %62
  %64 = fmul double %63, %61
  %65 = fmul double %64, 3.550000e-01
  %66 = fsub double 3.750000e+00, %65
  %67 = fmul double %66, %64
  %68 = fmul double %67, 3.550000e-01
  %69 = fsub double 3.750000e+00, %68
  %70 = fmul double %69, %67
  %71 = fmul double %70, 3.550000e-01
  %72 = fsub double 3.750000e+00, %71
  %73 = fmul double %72, %70
  %74 = fmul double %73, 3.550000e-01
  %75 = fsub double 3.750000e+00, %74
  %76 = fmul double %75, %73
  %77 = fmul double %76, 3.550000e-01
  %78 = fsub double 3.750000e+00, %77
  %79 = fmul double %78, %76
  %80 = fmul double %79, 3.550000e-01
  %81 = fsub double 3.750000e+00, %80
  %82 = fmul double %81, %79
  %83 = fmul double %82, 3.550000e-01
  %84 = fsub double 3.750000e+00, %83
  %85 = fmul double %84, %82
  %86 = fmul double %85, 3.550000e-01
  %87 = fsub double 3.750000e+00, %86
  %88 = fmul double %87, %85
  %89 = fmul double %88, 3.550000e-01
  %90 = fsub double 3.750000e+00, %89
  %91 = fmul double %90, %88
  %92 = fmul double %91, 3.550000e-01
  %93 = fsub double 3.750000e+00, %92
  %94 = fmul double %93, %91
  %95 = fmul double %94, 3.550000e-01
  %96 = fsub double 3.750000e+00, %95
  %97 = fmul double %96, %94
  %98 = fmul double %97, 3.550000e-01
  %99 = fsub double 3.750000e+00, %98
  %100 = fmul double %99, %97
  %101 = fmul double %100, 3.550000e-01
  %102 = fsub double 3.750000e+00, %101
  %103 = fmul double %102, %100
  %104 = fmul double %103, 3.550000e-01
  %105 = fsub double 3.750000e+00, %104
  %106 = fmul double %105, %103
  %107 = fmul double %106, 3.550000e-01
  %108 = fsub double 3.750000e+00, %107
  %109 = fmul double %108, %106
  %110 = fmul double %109, 3.550000e-01
  %111 = fsub double 3.750000e+00, %110
  %112 = fmul double %111, %109
  %113 = fmul double %112, 3.550000e-01
  %114 = fsub double 3.750000e+00, %113
  %115 = fmul double %114, %112
  %116 = fmul double %115, 3.550000e-01
  %117 = fsub double 3.750000e+00, %116
  %118 = fmul double %117, %115
  %119 = fmul double %118, 3.550000e-01
  %120 = fsub double 3.750000e+00, %119
  %121 = fmul double %120, %118
  %122 = fmul double %121, 3.550000e-01
  %123 = fsub double 3.750000e+00, %122
  %124 = fmul double %123, %121
  %125 = fmul double %124, 3.550000e-01
  %126 = fsub double 3.750000e+00, %125
  %127 = fmul double %126, %124
  %128 = fmul double %127, 3.550000e-01
  %129 = fsub double 3.750000e+00, %128
  %130 = fmul double %129, %127
  %131 = fmul double %130, 3.550000e-01
  %132 = fsub double 3.750000e+00, %131
  %133 = fmul double %132, %130
  %134 = fmul double %133, 3.550000e-01
  %135 = fsub double 3.750000e+00, %134
  %136 = fmul double %135, %133
  %137 = fmul double %136, 3.550000e-01
  %138 = fsub double 3.750000e+00, %137
  %139 = fmul double %138, %136
  %140 = fmul double %139, 3.550000e-01
  %141 = fsub double 3.750000e+00, %140
  %142 = fmul double %141, %139
  %143 = fmul double %142, 3.550000e-01
  %144 = fsub double 3.750000e+00, %143
  %145 = fmul double %144, %142
  %146 = fmul double %145, 3.550000e-01
  %147 = fsub double 3.750000e+00, %146
  %148 = fmul double %147, %145
  %149 = fmul double %148, 3.550000e-01
  %150 = fsub double 3.750000e+00, %149
  %151 = fmul double %150, %148
  %152 = fmul double %151, 3.550000e-01
  %153 = fsub double 3.750000e+00, %152
  %154 = fmul double %153, %151
  %155 = fmul double %154, 3.550000e-01
  %156 = fsub double 3.750000e+00, %155
  %157 = fmul double %156, %154
  %158 = fmul double %157, 3.550000e-01
  %159 = fsub double 3.750000e+00, %158
  %160 = fmul double %159, %157
  %161 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %161, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB3.i
  %s.0.lcssa.i = phi double [ %22, %SyncBB3.i ], [ %160, %bb.nph.i ]
  store double %s.0.lcssa.i, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MulMAdd1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__MulMAdd1_separated_args.exit:                   ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.MulMAdd1(i8* %pBuffer) {
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
  br label %SyncBB19.i

SyncBB19.i:                                       ; preds = %thenBB.i, %entry
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
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB19.i
  %j.02.i = phi i32 [ %160, %bb.nph.i ], [ 0, %SyncBB19.i ]
  %vectorPHI.i = phi <16 x double> [ %159, %bb.nph.i ], [ %21, %SyncBB19.i ]
  %22 = fmul <16 x double> %vectorPHI.i, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %23 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %22
  %24 = fmul <16 x double> %23, %vectorPHI.i
  %25 = fmul <16 x double> %24, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %26 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %25
  %27 = fmul <16 x double> %26, %24
  %28 = fmul <16 x double> %27, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %29 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %28
  %30 = fmul <16 x double> %29, %27
  %31 = fmul <16 x double> %30, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %32 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %31
  %33 = fmul <16 x double> %32, %30
  %34 = fmul <16 x double> %33, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %35 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %34
  %36 = fmul <16 x double> %35, %33
  %37 = fmul <16 x double> %36, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %38 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %37
  %39 = fmul <16 x double> %38, %36
  %40 = fmul <16 x double> %39, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %41 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %40
  %42 = fmul <16 x double> %41, %39
  %43 = fmul <16 x double> %42, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %44 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %43
  %45 = fmul <16 x double> %44, %42
  %46 = fmul <16 x double> %45, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %47 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %46
  %48 = fmul <16 x double> %47, %45
  %49 = fmul <16 x double> %48, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %50 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %49
  %51 = fmul <16 x double> %50, %48
  %52 = fmul <16 x double> %51, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %53 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %52
  %54 = fmul <16 x double> %53, %51
  %55 = fmul <16 x double> %54, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %56 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %55
  %57 = fmul <16 x double> %56, %54
  %58 = fmul <16 x double> %57, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %59 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %58
  %60 = fmul <16 x double> %59, %57
  %61 = fmul <16 x double> %60, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %62 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %61
  %63 = fmul <16 x double> %62, %60
  %64 = fmul <16 x double> %63, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %65 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %64
  %66 = fmul <16 x double> %65, %63
  %67 = fmul <16 x double> %66, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %68 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %67
  %69 = fmul <16 x double> %68, %66
  %70 = fmul <16 x double> %69, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %71 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %70
  %72 = fmul <16 x double> %71, %69
  %73 = fmul <16 x double> %72, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %74 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %73
  %75 = fmul <16 x double> %74, %72
  %76 = fmul <16 x double> %75, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %77 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %76
  %78 = fmul <16 x double> %77, %75
  %79 = fmul <16 x double> %78, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %80 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %79
  %81 = fmul <16 x double> %80, %78
  %82 = fmul <16 x double> %81, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %83 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %82
  %84 = fmul <16 x double> %83, %81
  %85 = fmul <16 x double> %84, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %86 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %85
  %87 = fmul <16 x double> %86, %84
  %88 = fmul <16 x double> %87, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %89 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %88
  %90 = fmul <16 x double> %89, %87
  %91 = fmul <16 x double> %90, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %92 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %91
  %93 = fmul <16 x double> %92, %90
  %94 = fmul <16 x double> %93, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %95 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %94
  %96 = fmul <16 x double> %95, %93
  %97 = fmul <16 x double> %96, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %98 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %97
  %99 = fmul <16 x double> %98, %96
  %100 = fmul <16 x double> %99, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %101 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %100
  %102 = fmul <16 x double> %101, %99
  %103 = fmul <16 x double> %102, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %104 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %103
  %105 = fmul <16 x double> %104, %102
  %106 = fmul <16 x double> %105, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %107 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %106
  %108 = fmul <16 x double> %107, %105
  %109 = fmul <16 x double> %108, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %110 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %109
  %111 = fmul <16 x double> %110, %108
  %112 = fmul <16 x double> %111, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %113 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %112
  %114 = fmul <16 x double> %113, %111
  %115 = fmul <16 x double> %114, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %116 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %115
  %117 = fmul <16 x double> %116, %114
  %118 = fmul <16 x double> %117, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %119 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %118
  %120 = fmul <16 x double> %119, %117
  %121 = fmul <16 x double> %120, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %122 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %121
  %123 = fmul <16 x double> %122, %120
  %124 = fmul <16 x double> %123, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %125 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %124
  %126 = fmul <16 x double> %125, %123
  %127 = fmul <16 x double> %126, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %128 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %127
  %129 = fmul <16 x double> %128, %126
  %130 = fmul <16 x double> %129, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %131 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %130
  %132 = fmul <16 x double> %131, %129
  %133 = fmul <16 x double> %132, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %134 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %133
  %135 = fmul <16 x double> %134, %132
  %136 = fmul <16 x double> %135, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %137 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %136
  %138 = fmul <16 x double> %137, %135
  %139 = fmul <16 x double> %138, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %140 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %139
  %141 = fmul <16 x double> %140, %138
  %142 = fmul <16 x double> %141, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %143 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %142
  %144 = fmul <16 x double> %143, %141
  %145 = fmul <16 x double> %144, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %146 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %145
  %147 = fmul <16 x double> %146, %144
  %148 = fmul <16 x double> %147, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %149 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %148
  %150 = fmul <16 x double> %149, %147
  %151 = fmul <16 x double> %150, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %152 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %151
  %153 = fmul <16 x double> %152, %150
  %154 = fmul <16 x double> %153, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %155 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %154
  %156 = fmul <16 x double> %155, %153
  %157 = fmul <16 x double> %156, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %158 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %157
  %159 = fmul <16 x double> %158, %156
  %160 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %160, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB19.i
  %vectorPHI17.i = phi <16 x double> [ %21, %SyncBB19.i ], [ %159, %bb.nph.i ]
  %ptrTypeCast18.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  store <16 x double> %vectorPHI17.i, <16 x double> addrspace(1)* %ptrTypeCast18.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.MulMAdd1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB19.i

____Vectorized_.MulMAdd1_separated_args.exit:     ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MulMAdd1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MulMAdd1_locals_anchor", void (i8*)* @MulMAdd1}
!1 = metadata !{i32 0, i32 0, i32 0}
