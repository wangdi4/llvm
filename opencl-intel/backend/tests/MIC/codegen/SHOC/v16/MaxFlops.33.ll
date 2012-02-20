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

declare void @__MulMAdd2_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.MulMAdd2_original(double addrspace(1)* nocapture, i32) nounwind

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

define void @__MulMAdd2_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB6

SyncBB6:                                          ; preds = %thenBB, %FirstBB
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
  %9 = fsub double 1.000000e+01, %8
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB6, %bb.nph
  %j.03 = phi i32 [ %148, %bb.nph ], [ 0, %SyncBB6 ]
  %s2.02 = phi double [ %147, %bb.nph ], [ %9, %SyncBB6 ]
  %s.01 = phi double [ %144, %bb.nph ], [ %8, %SyncBB6 ]
  %10 = fmul double %s.01, 3.550000e-01
  %11 = fsub double 3.750000e+00, %10
  %12 = fmul double %11, %s.01
  %13 = fmul double %s2.02, 3.550000e-01
  %14 = fsub double 3.750000e+00, %13
  %15 = fmul double %14, %s2.02
  %16 = fmul double %12, 3.550000e-01
  %17 = fsub double 3.750000e+00, %16
  %18 = fmul double %17, %12
  %19 = fmul double %15, 3.550000e-01
  %20 = fsub double 3.750000e+00, %19
  %21 = fmul double %20, %15
  %22 = fmul double %18, 3.550000e-01
  %23 = fsub double 3.750000e+00, %22
  %24 = fmul double %23, %18
  %25 = fmul double %21, 3.550000e-01
  %26 = fsub double 3.750000e+00, %25
  %27 = fmul double %26, %21
  %28 = fmul double %24, 3.550000e-01
  %29 = fsub double 3.750000e+00, %28
  %30 = fmul double %29, %24
  %31 = fmul double %27, 3.550000e-01
  %32 = fsub double 3.750000e+00, %31
  %33 = fmul double %32, %27
  %34 = fmul double %30, 3.550000e-01
  %35 = fsub double 3.750000e+00, %34
  %36 = fmul double %35, %30
  %37 = fmul double %33, 3.550000e-01
  %38 = fsub double 3.750000e+00, %37
  %39 = fmul double %38, %33
  %40 = fmul double %36, 3.550000e-01
  %41 = fsub double 3.750000e+00, %40
  %42 = fmul double %41, %36
  %43 = fmul double %39, 3.550000e-01
  %44 = fsub double 3.750000e+00, %43
  %45 = fmul double %44, %39
  %46 = fmul double %42, 3.550000e-01
  %47 = fsub double 3.750000e+00, %46
  %48 = fmul double %47, %42
  %49 = fmul double %45, 3.550000e-01
  %50 = fsub double 3.750000e+00, %49
  %51 = fmul double %50, %45
  %52 = fmul double %48, 3.550000e-01
  %53 = fsub double 3.750000e+00, %52
  %54 = fmul double %53, %48
  %55 = fmul double %51, 3.550000e-01
  %56 = fsub double 3.750000e+00, %55
  %57 = fmul double %56, %51
  %58 = fmul double %54, 3.550000e-01
  %59 = fsub double 3.750000e+00, %58
  %60 = fmul double %59, %54
  %61 = fmul double %57, 3.550000e-01
  %62 = fsub double 3.750000e+00, %61
  %63 = fmul double %62, %57
  %64 = fmul double %60, 3.550000e-01
  %65 = fsub double 3.750000e+00, %64
  %66 = fmul double %65, %60
  %67 = fmul double %63, 3.550000e-01
  %68 = fsub double 3.750000e+00, %67
  %69 = fmul double %68, %63
  %70 = fmul double %66, 3.550000e-01
  %71 = fsub double 3.750000e+00, %70
  %72 = fmul double %71, %66
  %73 = fmul double %69, 3.550000e-01
  %74 = fsub double 3.750000e+00, %73
  %75 = fmul double %74, %69
  %76 = fmul double %72, 3.550000e-01
  %77 = fsub double 3.750000e+00, %76
  %78 = fmul double %77, %72
  %79 = fmul double %75, 3.550000e-01
  %80 = fsub double 3.750000e+00, %79
  %81 = fmul double %80, %75
  %82 = fmul double %78, 3.550000e-01
  %83 = fsub double 3.750000e+00, %82
  %84 = fmul double %83, %78
  %85 = fmul double %81, 3.550000e-01
  %86 = fsub double 3.750000e+00, %85
  %87 = fmul double %86, %81
  %88 = fmul double %84, 3.550000e-01
  %89 = fsub double 3.750000e+00, %88
  %90 = fmul double %89, %84
  %91 = fmul double %87, 3.550000e-01
  %92 = fsub double 3.750000e+00, %91
  %93 = fmul double %92, %87
  %94 = fmul double %90, 3.550000e-01
  %95 = fsub double 3.750000e+00, %94
  %96 = fmul double %95, %90
  %97 = fmul double %93, 3.550000e-01
  %98 = fsub double 3.750000e+00, %97
  %99 = fmul double %98, %93
  %100 = fmul double %96, 3.550000e-01
  %101 = fsub double 3.750000e+00, %100
  %102 = fmul double %101, %96
  %103 = fmul double %99, 3.550000e-01
  %104 = fsub double 3.750000e+00, %103
  %105 = fmul double %104, %99
  %106 = fmul double %102, 3.550000e-01
  %107 = fsub double 3.750000e+00, %106
  %108 = fmul double %107, %102
  %109 = fmul double %105, 3.550000e-01
  %110 = fsub double 3.750000e+00, %109
  %111 = fmul double %110, %105
  %112 = fmul double %108, 3.550000e-01
  %113 = fsub double 3.750000e+00, %112
  %114 = fmul double %113, %108
  %115 = fmul double %111, 3.550000e-01
  %116 = fsub double 3.750000e+00, %115
  %117 = fmul double %116, %111
  %118 = fmul double %114, 3.550000e-01
  %119 = fsub double 3.750000e+00, %118
  %120 = fmul double %119, %114
  %121 = fmul double %117, 3.550000e-01
  %122 = fsub double 3.750000e+00, %121
  %123 = fmul double %122, %117
  %124 = fmul double %120, 3.550000e-01
  %125 = fsub double 3.750000e+00, %124
  %126 = fmul double %125, %120
  %127 = fmul double %123, 3.550000e-01
  %128 = fsub double 3.750000e+00, %127
  %129 = fmul double %128, %123
  %130 = fmul double %126, 3.550000e-01
  %131 = fsub double 3.750000e+00, %130
  %132 = fmul double %131, %126
  %133 = fmul double %129, 3.550000e-01
  %134 = fsub double 3.750000e+00, %133
  %135 = fmul double %134, %129
  %136 = fmul double %132, 3.550000e-01
  %137 = fsub double 3.750000e+00, %136
  %138 = fmul double %137, %132
  %139 = fmul double %135, 3.550000e-01
  %140 = fsub double 3.750000e+00, %139
  %141 = fmul double %140, %135
  %142 = fmul double %138, 3.550000e-01
  %143 = fsub double 3.750000e+00, %142
  %144 = fmul double %143, %138
  %145 = fmul double %141, 3.550000e-01
  %146 = fsub double 3.750000e+00, %145
  %147 = fmul double %146, %141
  %148 = add nsw i32 %j.03, 1
  %exitcond = icmp eq i32 %148, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB6
  %s2.0.lcssa = phi double [ %9, %SyncBB6 ], [ %147, %bb.nph ]
  %s.0.lcssa = phi double [ %8, %SyncBB6 ], [ %144, %bb.nph ]
  %149 = fadd double %s.0.lcssa, %s2.0.lcssa
  store double %149, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB6

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.MulMAdd2_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %6 = getelementptr inbounds double addrspace(1)* %data, i64 %extract
  %ptrTypeCast = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  %7 = load <16 x double> addrspace(1)* %ptrTypeCast, align 8
  %8 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %7
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %j.03 = phi i32 [ %147, %bb.nph ], [ 0, %SyncBB ]
  %vectorPHI = phi <16 x double> [ %146, %bb.nph ], [ %8, %SyncBB ]
  %vectorPHI17 = phi <16 x double> [ %143, %bb.nph ], [ %7, %SyncBB ]
  %9 = fmul <16 x double> %vectorPHI17, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %10 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %9
  %11 = fmul <16 x double> %10, %vectorPHI17
  %12 = fmul <16 x double> %vectorPHI, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %13 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %12
  %14 = fmul <16 x double> %13, %vectorPHI
  %15 = fmul <16 x double> %11, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %16 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %15
  %17 = fmul <16 x double> %16, %11
  %18 = fmul <16 x double> %14, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %19 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %18
  %20 = fmul <16 x double> %19, %14
  %21 = fmul <16 x double> %17, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %22 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %21
  %23 = fmul <16 x double> %22, %17
  %24 = fmul <16 x double> %20, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %25 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %24
  %26 = fmul <16 x double> %25, %20
  %27 = fmul <16 x double> %23, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %28 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %27
  %29 = fmul <16 x double> %28, %23
  %30 = fmul <16 x double> %26, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %31 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %30
  %32 = fmul <16 x double> %31, %26
  %33 = fmul <16 x double> %29, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %34 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %33
  %35 = fmul <16 x double> %34, %29
  %36 = fmul <16 x double> %32, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %37 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %36
  %38 = fmul <16 x double> %37, %32
  %39 = fmul <16 x double> %35, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %40 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %39
  %41 = fmul <16 x double> %40, %35
  %42 = fmul <16 x double> %38, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %43 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %42
  %44 = fmul <16 x double> %43, %38
  %45 = fmul <16 x double> %41, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %46 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %45
  %47 = fmul <16 x double> %46, %41
  %48 = fmul <16 x double> %44, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %49 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %48
  %50 = fmul <16 x double> %49, %44
  %51 = fmul <16 x double> %47, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %52 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %51
  %53 = fmul <16 x double> %52, %47
  %54 = fmul <16 x double> %50, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %55 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %54
  %56 = fmul <16 x double> %55, %50
  %57 = fmul <16 x double> %53, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %58 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %57
  %59 = fmul <16 x double> %58, %53
  %60 = fmul <16 x double> %56, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %61 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %60
  %62 = fmul <16 x double> %61, %56
  %63 = fmul <16 x double> %59, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %64 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %63
  %65 = fmul <16 x double> %64, %59
  %66 = fmul <16 x double> %62, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %67 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %66
  %68 = fmul <16 x double> %67, %62
  %69 = fmul <16 x double> %65, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %70 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %69
  %71 = fmul <16 x double> %70, %65
  %72 = fmul <16 x double> %68, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %73 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %72
  %74 = fmul <16 x double> %73, %68
  %75 = fmul <16 x double> %71, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %76 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %75
  %77 = fmul <16 x double> %76, %71
  %78 = fmul <16 x double> %74, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %79 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %78
  %80 = fmul <16 x double> %79, %74
  %81 = fmul <16 x double> %77, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %82 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %81
  %83 = fmul <16 x double> %82, %77
  %84 = fmul <16 x double> %80, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %85 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %84
  %86 = fmul <16 x double> %85, %80
  %87 = fmul <16 x double> %83, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %88 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %87
  %89 = fmul <16 x double> %88, %83
  %90 = fmul <16 x double> %86, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %91 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %90
  %92 = fmul <16 x double> %91, %86
  %93 = fmul <16 x double> %89, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %94 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %93
  %95 = fmul <16 x double> %94, %89
  %96 = fmul <16 x double> %92, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %97 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %96
  %98 = fmul <16 x double> %97, %92
  %99 = fmul <16 x double> %95, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %100 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %99
  %101 = fmul <16 x double> %100, %95
  %102 = fmul <16 x double> %98, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %103 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %102
  %104 = fmul <16 x double> %103, %98
  %105 = fmul <16 x double> %101, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %106 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %105
  %107 = fmul <16 x double> %106, %101
  %108 = fmul <16 x double> %104, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %109 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %108
  %110 = fmul <16 x double> %109, %104
  %111 = fmul <16 x double> %107, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %112 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %111
  %113 = fmul <16 x double> %112, %107
  %114 = fmul <16 x double> %110, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %115 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %114
  %116 = fmul <16 x double> %115, %110
  %117 = fmul <16 x double> %113, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %118 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %117
  %119 = fmul <16 x double> %118, %113
  %120 = fmul <16 x double> %116, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %121 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %120
  %122 = fmul <16 x double> %121, %116
  %123 = fmul <16 x double> %119, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %124 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %123
  %125 = fmul <16 x double> %124, %119
  %126 = fmul <16 x double> %122, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %127 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %126
  %128 = fmul <16 x double> %127, %122
  %129 = fmul <16 x double> %125, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %130 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %129
  %131 = fmul <16 x double> %130, %125
  %132 = fmul <16 x double> %128, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %133 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %132
  %134 = fmul <16 x double> %133, %128
  %135 = fmul <16 x double> %131, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %136 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %135
  %137 = fmul <16 x double> %136, %131
  %138 = fmul <16 x double> %134, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %139 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %138
  %140 = fmul <16 x double> %139, %134
  %141 = fmul <16 x double> %137, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %142 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %141
  %143 = fmul <16 x double> %142, %137
  %144 = fmul <16 x double> %140, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %145 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %144
  %146 = fmul <16 x double> %145, %140
  %147 = add nsw i32 %j.03, 1
  %exitcond = icmp eq i32 %147, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %vectorPHI18 = phi <16 x double> [ %8, %SyncBB ], [ %146, %bb.nph ]
  %vectorPHI19 = phi <16 x double> [ %7, %SyncBB ], [ %143, %bb.nph ]
  %148 = fadd <16 x double> %vectorPHI19, %vectorPHI18
  %ptrTypeCast20 = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  store <16 x double> %148, <16 x double> addrspace(1)* %ptrTypeCast20, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB21

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB21:                                         ; preds = %._crit_edge
  ret void
}

define void @MulMAdd2(i8* %pBuffer) {
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
  br label %SyncBB6.i

SyncBB6.i:                                        ; preds = %thenBB.i, %entry
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
  %23 = fsub double 1.000000e+01, %22
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB6.i
  %j.03.i = phi i32 [ %162, %bb.nph.i ], [ 0, %SyncBB6.i ]
  %s2.02.i = phi double [ %161, %bb.nph.i ], [ %23, %SyncBB6.i ]
  %s.01.i = phi double [ %158, %bb.nph.i ], [ %22, %SyncBB6.i ]
  %24 = fmul double %s.01.i, 3.550000e-01
  %25 = fsub double 3.750000e+00, %24
  %26 = fmul double %25, %s.01.i
  %27 = fmul double %s2.02.i, 3.550000e-01
  %28 = fsub double 3.750000e+00, %27
  %29 = fmul double %28, %s2.02.i
  %30 = fmul double %26, 3.550000e-01
  %31 = fsub double 3.750000e+00, %30
  %32 = fmul double %31, %26
  %33 = fmul double %29, 3.550000e-01
  %34 = fsub double 3.750000e+00, %33
  %35 = fmul double %34, %29
  %36 = fmul double %32, 3.550000e-01
  %37 = fsub double 3.750000e+00, %36
  %38 = fmul double %37, %32
  %39 = fmul double %35, 3.550000e-01
  %40 = fsub double 3.750000e+00, %39
  %41 = fmul double %40, %35
  %42 = fmul double %38, 3.550000e-01
  %43 = fsub double 3.750000e+00, %42
  %44 = fmul double %43, %38
  %45 = fmul double %41, 3.550000e-01
  %46 = fsub double 3.750000e+00, %45
  %47 = fmul double %46, %41
  %48 = fmul double %44, 3.550000e-01
  %49 = fsub double 3.750000e+00, %48
  %50 = fmul double %49, %44
  %51 = fmul double %47, 3.550000e-01
  %52 = fsub double 3.750000e+00, %51
  %53 = fmul double %52, %47
  %54 = fmul double %50, 3.550000e-01
  %55 = fsub double 3.750000e+00, %54
  %56 = fmul double %55, %50
  %57 = fmul double %53, 3.550000e-01
  %58 = fsub double 3.750000e+00, %57
  %59 = fmul double %58, %53
  %60 = fmul double %56, 3.550000e-01
  %61 = fsub double 3.750000e+00, %60
  %62 = fmul double %61, %56
  %63 = fmul double %59, 3.550000e-01
  %64 = fsub double 3.750000e+00, %63
  %65 = fmul double %64, %59
  %66 = fmul double %62, 3.550000e-01
  %67 = fsub double 3.750000e+00, %66
  %68 = fmul double %67, %62
  %69 = fmul double %65, 3.550000e-01
  %70 = fsub double 3.750000e+00, %69
  %71 = fmul double %70, %65
  %72 = fmul double %68, 3.550000e-01
  %73 = fsub double 3.750000e+00, %72
  %74 = fmul double %73, %68
  %75 = fmul double %71, 3.550000e-01
  %76 = fsub double 3.750000e+00, %75
  %77 = fmul double %76, %71
  %78 = fmul double %74, 3.550000e-01
  %79 = fsub double 3.750000e+00, %78
  %80 = fmul double %79, %74
  %81 = fmul double %77, 3.550000e-01
  %82 = fsub double 3.750000e+00, %81
  %83 = fmul double %82, %77
  %84 = fmul double %80, 3.550000e-01
  %85 = fsub double 3.750000e+00, %84
  %86 = fmul double %85, %80
  %87 = fmul double %83, 3.550000e-01
  %88 = fsub double 3.750000e+00, %87
  %89 = fmul double %88, %83
  %90 = fmul double %86, 3.550000e-01
  %91 = fsub double 3.750000e+00, %90
  %92 = fmul double %91, %86
  %93 = fmul double %89, 3.550000e-01
  %94 = fsub double 3.750000e+00, %93
  %95 = fmul double %94, %89
  %96 = fmul double %92, 3.550000e-01
  %97 = fsub double 3.750000e+00, %96
  %98 = fmul double %97, %92
  %99 = fmul double %95, 3.550000e-01
  %100 = fsub double 3.750000e+00, %99
  %101 = fmul double %100, %95
  %102 = fmul double %98, 3.550000e-01
  %103 = fsub double 3.750000e+00, %102
  %104 = fmul double %103, %98
  %105 = fmul double %101, 3.550000e-01
  %106 = fsub double 3.750000e+00, %105
  %107 = fmul double %106, %101
  %108 = fmul double %104, 3.550000e-01
  %109 = fsub double 3.750000e+00, %108
  %110 = fmul double %109, %104
  %111 = fmul double %107, 3.550000e-01
  %112 = fsub double 3.750000e+00, %111
  %113 = fmul double %112, %107
  %114 = fmul double %110, 3.550000e-01
  %115 = fsub double 3.750000e+00, %114
  %116 = fmul double %115, %110
  %117 = fmul double %113, 3.550000e-01
  %118 = fsub double 3.750000e+00, %117
  %119 = fmul double %118, %113
  %120 = fmul double %116, 3.550000e-01
  %121 = fsub double 3.750000e+00, %120
  %122 = fmul double %121, %116
  %123 = fmul double %119, 3.550000e-01
  %124 = fsub double 3.750000e+00, %123
  %125 = fmul double %124, %119
  %126 = fmul double %122, 3.550000e-01
  %127 = fsub double 3.750000e+00, %126
  %128 = fmul double %127, %122
  %129 = fmul double %125, 3.550000e-01
  %130 = fsub double 3.750000e+00, %129
  %131 = fmul double %130, %125
  %132 = fmul double %128, 3.550000e-01
  %133 = fsub double 3.750000e+00, %132
  %134 = fmul double %133, %128
  %135 = fmul double %131, 3.550000e-01
  %136 = fsub double 3.750000e+00, %135
  %137 = fmul double %136, %131
  %138 = fmul double %134, 3.550000e-01
  %139 = fsub double 3.750000e+00, %138
  %140 = fmul double %139, %134
  %141 = fmul double %137, 3.550000e-01
  %142 = fsub double 3.750000e+00, %141
  %143 = fmul double %142, %137
  %144 = fmul double %140, 3.550000e-01
  %145 = fsub double 3.750000e+00, %144
  %146 = fmul double %145, %140
  %147 = fmul double %143, 3.550000e-01
  %148 = fsub double 3.750000e+00, %147
  %149 = fmul double %148, %143
  %150 = fmul double %146, 3.550000e-01
  %151 = fsub double 3.750000e+00, %150
  %152 = fmul double %151, %146
  %153 = fmul double %149, 3.550000e-01
  %154 = fsub double 3.750000e+00, %153
  %155 = fmul double %154, %149
  %156 = fmul double %152, 3.550000e-01
  %157 = fsub double 3.750000e+00, %156
  %158 = fmul double %157, %152
  %159 = fmul double %155, 3.550000e-01
  %160 = fsub double 3.750000e+00, %159
  %161 = fmul double %160, %155
  %162 = add nsw i32 %j.03.i, 1
  %exitcond.i = icmp eq i32 %162, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB6.i
  %s2.0.lcssa.i = phi double [ %23, %SyncBB6.i ], [ %161, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %22, %SyncBB6.i ], [ %158, %bb.nph.i ]
  %163 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  store double %163, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MulMAdd2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB6.i

__MulMAdd2_separated_args.exit:                   ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.MulMAdd2(i8* %pBuffer) {
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
  %extract.lhs.lhs.i = shl i64 %19, 32
  %extract.i = ashr i64 %extract.lhs.lhs.i, 32
  %20 = getelementptr inbounds double addrspace(1)* %1, i64 %extract.i
  %ptrTypeCast.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  %21 = load <16 x double> addrspace(1)* %ptrTypeCast.i, align 8
  %22 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %21
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %j.03.i = phi i32 [ %161, %bb.nph.i ], [ 0, %SyncBB.i ]
  %vectorPHI.i = phi <16 x double> [ %160, %bb.nph.i ], [ %22, %SyncBB.i ]
  %vectorPHI17.i = phi <16 x double> [ %157, %bb.nph.i ], [ %21, %SyncBB.i ]
  %23 = fmul <16 x double> %vectorPHI17.i, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %24 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %23
  %25 = fmul <16 x double> %24, %vectorPHI17.i
  %26 = fmul <16 x double> %vectorPHI.i, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %27 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %26
  %28 = fmul <16 x double> %27, %vectorPHI.i
  %29 = fmul <16 x double> %25, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %30 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %29
  %31 = fmul <16 x double> %30, %25
  %32 = fmul <16 x double> %28, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %33 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %32
  %34 = fmul <16 x double> %33, %28
  %35 = fmul <16 x double> %31, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %36 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %35
  %37 = fmul <16 x double> %36, %31
  %38 = fmul <16 x double> %34, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %39 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %38
  %40 = fmul <16 x double> %39, %34
  %41 = fmul <16 x double> %37, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %42 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %41
  %43 = fmul <16 x double> %42, %37
  %44 = fmul <16 x double> %40, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %45 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %44
  %46 = fmul <16 x double> %45, %40
  %47 = fmul <16 x double> %43, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %48 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %47
  %49 = fmul <16 x double> %48, %43
  %50 = fmul <16 x double> %46, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %51 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %50
  %52 = fmul <16 x double> %51, %46
  %53 = fmul <16 x double> %49, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %54 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %53
  %55 = fmul <16 x double> %54, %49
  %56 = fmul <16 x double> %52, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %57 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %56
  %58 = fmul <16 x double> %57, %52
  %59 = fmul <16 x double> %55, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %60 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %59
  %61 = fmul <16 x double> %60, %55
  %62 = fmul <16 x double> %58, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %63 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %62
  %64 = fmul <16 x double> %63, %58
  %65 = fmul <16 x double> %61, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %66 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %65
  %67 = fmul <16 x double> %66, %61
  %68 = fmul <16 x double> %64, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %69 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %68
  %70 = fmul <16 x double> %69, %64
  %71 = fmul <16 x double> %67, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %72 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %71
  %73 = fmul <16 x double> %72, %67
  %74 = fmul <16 x double> %70, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %75 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %74
  %76 = fmul <16 x double> %75, %70
  %77 = fmul <16 x double> %73, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %78 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %77
  %79 = fmul <16 x double> %78, %73
  %80 = fmul <16 x double> %76, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %81 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %80
  %82 = fmul <16 x double> %81, %76
  %83 = fmul <16 x double> %79, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %84 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %83
  %85 = fmul <16 x double> %84, %79
  %86 = fmul <16 x double> %82, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %87 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %86
  %88 = fmul <16 x double> %87, %82
  %89 = fmul <16 x double> %85, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %90 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %89
  %91 = fmul <16 x double> %90, %85
  %92 = fmul <16 x double> %88, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %93 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %92
  %94 = fmul <16 x double> %93, %88
  %95 = fmul <16 x double> %91, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %96 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %95
  %97 = fmul <16 x double> %96, %91
  %98 = fmul <16 x double> %94, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %99 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %98
  %100 = fmul <16 x double> %99, %94
  %101 = fmul <16 x double> %97, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %102 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %101
  %103 = fmul <16 x double> %102, %97
  %104 = fmul <16 x double> %100, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %105 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %104
  %106 = fmul <16 x double> %105, %100
  %107 = fmul <16 x double> %103, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %108 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %107
  %109 = fmul <16 x double> %108, %103
  %110 = fmul <16 x double> %106, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %111 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %110
  %112 = fmul <16 x double> %111, %106
  %113 = fmul <16 x double> %109, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %114 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %113
  %115 = fmul <16 x double> %114, %109
  %116 = fmul <16 x double> %112, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %117 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %116
  %118 = fmul <16 x double> %117, %112
  %119 = fmul <16 x double> %115, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %120 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %119
  %121 = fmul <16 x double> %120, %115
  %122 = fmul <16 x double> %118, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %123 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %122
  %124 = fmul <16 x double> %123, %118
  %125 = fmul <16 x double> %121, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %126 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %125
  %127 = fmul <16 x double> %126, %121
  %128 = fmul <16 x double> %124, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %129 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %128
  %130 = fmul <16 x double> %129, %124
  %131 = fmul <16 x double> %127, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %132 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %131
  %133 = fmul <16 x double> %132, %127
  %134 = fmul <16 x double> %130, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %135 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %134
  %136 = fmul <16 x double> %135, %130
  %137 = fmul <16 x double> %133, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %138 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %137
  %139 = fmul <16 x double> %138, %133
  %140 = fmul <16 x double> %136, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %141 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %140
  %142 = fmul <16 x double> %141, %136
  %143 = fmul <16 x double> %139, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %144 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %143
  %145 = fmul <16 x double> %144, %139
  %146 = fmul <16 x double> %142, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %147 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %146
  %148 = fmul <16 x double> %147, %142
  %149 = fmul <16 x double> %145, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %150 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %149
  %151 = fmul <16 x double> %150, %145
  %152 = fmul <16 x double> %148, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %153 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %152
  %154 = fmul <16 x double> %153, %148
  %155 = fmul <16 x double> %151, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %156 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %155
  %157 = fmul <16 x double> %156, %151
  %158 = fmul <16 x double> %154, <double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01, double 3.550000e-01>
  %159 = fsub <16 x double> <double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00, double 3.750000e+00>, %158
  %160 = fmul <16 x double> %159, %154
  %161 = add nsw i32 %j.03.i, 1
  %exitcond.i = icmp eq i32 %161, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %vectorPHI18.i = phi <16 x double> [ %22, %SyncBB.i ], [ %160, %bb.nph.i ]
  %vectorPHI19.i = phi <16 x double> [ %21, %SyncBB.i ], [ %157, %bb.nph.i ]
  %162 = fadd <16 x double> %vectorPHI19.i, %vectorPHI18.i
  %ptrTypeCast20.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  store <16 x double> %162, <16 x double> addrspace(1)* %ptrTypeCast20.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.MulMAdd2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.MulMAdd2_separated_args.exit:     ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MulMAdd2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MulMAdd2_locals_anchor", void (i8*)* @MulMAdd2}
!1 = metadata !{i32 0, i32 0, i32 0}
