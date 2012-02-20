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

declare void @__MAdd8_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @____Vectorized_.MAdd8_original(double addrspace(1)* nocapture, i32) nounwind

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

define void @__MAdd8_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %9 = fsub double 1.000000e+01, %8
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %j.09 = phi i32 [ %154, %bb.nph ], [ 0, %SyncBB ]
  %s8.08 = phi double [ %153, %bb.nph ], [ %9, %SyncBB ]
  %s7.07 = phi double [ %151, %bb.nph ], [ %8, %SyncBB ]
  %s6.06 = phi double [ %149, %bb.nph ], [ %9, %SyncBB ]
  %s.05 = phi double [ %139, %bb.nph ], [ %8, %SyncBB ]
  %s2.04 = phi double [ %141, %bb.nph ], [ %9, %SyncBB ]
  %s3.03 = phi double [ %143, %bb.nph ], [ %8, %SyncBB ]
  %s4.02 = phi double [ %145, %bb.nph ], [ %9, %SyncBB ]
  %s5.01 = phi double [ %147, %bb.nph ], [ %8, %SyncBB ]
  %10 = fmul double %s.05, 9.899000e-01
  %11 = fsub double 1.000000e+01, %10
  %12 = fmul double %s2.04, 9.899000e-01
  %13 = fsub double 1.000000e+01, %12
  %14 = fmul double %s3.03, 9.899000e-01
  %15 = fsub double 1.000000e+01, %14
  %16 = fmul double %s4.02, 9.899000e-01
  %17 = fsub double 1.000000e+01, %16
  %18 = fmul double %s5.01, 9.899000e-01
  %19 = fsub double 1.000000e+01, %18
  %20 = fmul double %s6.06, 9.899000e-01
  %21 = fsub double 1.000000e+01, %20
  %22 = fmul double %s7.07, 9.899000e-01
  %23 = fsub double 1.000000e+01, %22
  %24 = fmul double %s8.08, 9.899000e-01
  %25 = fsub double 1.000000e+01, %24
  %26 = fmul double %11, 9.899000e-01
  %27 = fsub double 1.000000e+01, %26
  %28 = fmul double %13, 9.899000e-01
  %29 = fsub double 1.000000e+01, %28
  %30 = fmul double %15, 9.899000e-01
  %31 = fsub double 1.000000e+01, %30
  %32 = fmul double %17, 9.899000e-01
  %33 = fsub double 1.000000e+01, %32
  %34 = fmul double %19, 9.899000e-01
  %35 = fsub double 1.000000e+01, %34
  %36 = fmul double %21, 9.899000e-01
  %37 = fsub double 1.000000e+01, %36
  %38 = fmul double %23, 9.899000e-01
  %39 = fsub double 1.000000e+01, %38
  %40 = fmul double %25, 9.899000e-01
  %41 = fsub double 1.000000e+01, %40
  %42 = fmul double %27, 9.899000e-01
  %43 = fsub double 1.000000e+01, %42
  %44 = fmul double %29, 9.899000e-01
  %45 = fsub double 1.000000e+01, %44
  %46 = fmul double %31, 9.899000e-01
  %47 = fsub double 1.000000e+01, %46
  %48 = fmul double %33, 9.899000e-01
  %49 = fsub double 1.000000e+01, %48
  %50 = fmul double %35, 9.899000e-01
  %51 = fsub double 1.000000e+01, %50
  %52 = fmul double %37, 9.899000e-01
  %53 = fsub double 1.000000e+01, %52
  %54 = fmul double %39, 9.899000e-01
  %55 = fsub double 1.000000e+01, %54
  %56 = fmul double %41, 9.899000e-01
  %57 = fsub double 1.000000e+01, %56
  %58 = fmul double %43, 9.899000e-01
  %59 = fsub double 1.000000e+01, %58
  %60 = fmul double %45, 9.899000e-01
  %61 = fsub double 1.000000e+01, %60
  %62 = fmul double %47, 9.899000e-01
  %63 = fsub double 1.000000e+01, %62
  %64 = fmul double %49, 9.899000e-01
  %65 = fsub double 1.000000e+01, %64
  %66 = fmul double %51, 9.899000e-01
  %67 = fsub double 1.000000e+01, %66
  %68 = fmul double %53, 9.899000e-01
  %69 = fsub double 1.000000e+01, %68
  %70 = fmul double %55, 9.899000e-01
  %71 = fsub double 1.000000e+01, %70
  %72 = fmul double %57, 9.899000e-01
  %73 = fsub double 1.000000e+01, %72
  %74 = fmul double %59, 9.899000e-01
  %75 = fsub double 1.000000e+01, %74
  %76 = fmul double %61, 9.899000e-01
  %77 = fsub double 1.000000e+01, %76
  %78 = fmul double %63, 9.899000e-01
  %79 = fsub double 1.000000e+01, %78
  %80 = fmul double %65, 9.899000e-01
  %81 = fsub double 1.000000e+01, %80
  %82 = fmul double %67, 9.899000e-01
  %83 = fsub double 1.000000e+01, %82
  %84 = fmul double %69, 9.899000e-01
  %85 = fsub double 1.000000e+01, %84
  %86 = fmul double %71, 9.899000e-01
  %87 = fsub double 1.000000e+01, %86
  %88 = fmul double %73, 9.899000e-01
  %89 = fsub double 1.000000e+01, %88
  %90 = fmul double %75, 9.899000e-01
  %91 = fsub double 1.000000e+01, %90
  %92 = fmul double %77, 9.899000e-01
  %93 = fsub double 1.000000e+01, %92
  %94 = fmul double %79, 9.899000e-01
  %95 = fsub double 1.000000e+01, %94
  %96 = fmul double %81, 9.899000e-01
  %97 = fsub double 1.000000e+01, %96
  %98 = fmul double %83, 9.899000e-01
  %99 = fsub double 1.000000e+01, %98
  %100 = fmul double %85, 9.899000e-01
  %101 = fsub double 1.000000e+01, %100
  %102 = fmul double %87, 9.899000e-01
  %103 = fsub double 1.000000e+01, %102
  %104 = fmul double %89, 9.899000e-01
  %105 = fsub double 1.000000e+01, %104
  %106 = fmul double %91, 9.899000e-01
  %107 = fsub double 1.000000e+01, %106
  %108 = fmul double %93, 9.899000e-01
  %109 = fsub double 1.000000e+01, %108
  %110 = fmul double %95, 9.899000e-01
  %111 = fsub double 1.000000e+01, %110
  %112 = fmul double %97, 9.899000e-01
  %113 = fsub double 1.000000e+01, %112
  %114 = fmul double %99, 9.899000e-01
  %115 = fsub double 1.000000e+01, %114
  %116 = fmul double %101, 9.899000e-01
  %117 = fsub double 1.000000e+01, %116
  %118 = fmul double %103, 9.899000e-01
  %119 = fsub double 1.000000e+01, %118
  %120 = fmul double %105, 9.899000e-01
  %121 = fsub double 1.000000e+01, %120
  %122 = fmul double %107, 9.899000e-01
  %123 = fsub double 1.000000e+01, %122
  %124 = fmul double %109, 9.899000e-01
  %125 = fsub double 1.000000e+01, %124
  %126 = fmul double %111, 9.899000e-01
  %127 = fsub double 1.000000e+01, %126
  %128 = fmul double %113, 9.899000e-01
  %129 = fsub double 1.000000e+01, %128
  %130 = fmul double %115, 9.899000e-01
  %131 = fsub double 1.000000e+01, %130
  %132 = fmul double %117, 9.899000e-01
  %133 = fsub double 1.000000e+01, %132
  %134 = fmul double %119, 9.899000e-01
  %135 = fsub double 1.000000e+01, %134
  %136 = fmul double %121, 9.899000e-01
  %137 = fsub double 1.000000e+01, %136
  %138 = fmul double %123, 9.899000e-01
  %139 = fsub double 1.000000e+01, %138
  %140 = fmul double %125, 9.899000e-01
  %141 = fsub double 1.000000e+01, %140
  %142 = fmul double %127, 9.899000e-01
  %143 = fsub double 1.000000e+01, %142
  %144 = fmul double %129, 9.899000e-01
  %145 = fsub double 1.000000e+01, %144
  %146 = fmul double %131, 9.899000e-01
  %147 = fsub double 1.000000e+01, %146
  %148 = fmul double %133, 9.899000e-01
  %149 = fsub double 1.000000e+01, %148
  %150 = fmul double %135, 9.899000e-01
  %151 = fsub double 1.000000e+01, %150
  %152 = fmul double %137, 9.899000e-01
  %153 = fsub double 1.000000e+01, %152
  %154 = add nsw i32 %j.09, 1
  %exitcond = icmp eq i32 %154, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %s8.0.lcssa = phi double [ %9, %SyncBB ], [ %153, %bb.nph ]
  %s7.0.lcssa = phi double [ %8, %SyncBB ], [ %151, %bb.nph ]
  %s6.0.lcssa = phi double [ %9, %SyncBB ], [ %149, %bb.nph ]
  %s.0.lcssa = phi double [ %8, %SyncBB ], [ %139, %bb.nph ]
  %s2.0.lcssa = phi double [ %9, %SyncBB ], [ %141, %bb.nph ]
  %s3.0.lcssa = phi double [ %8, %SyncBB ], [ %143, %bb.nph ]
  %s4.0.lcssa = phi double [ %9, %SyncBB ], [ %145, %bb.nph ]
  %s5.0.lcssa = phi double [ %8, %SyncBB ], [ %147, %bb.nph ]
  %155 = fadd double %s.0.lcssa, %s2.0.lcssa
  %156 = fadd double %155, %s3.0.lcssa
  %157 = fadd double %156, %s4.0.lcssa
  %158 = fadd double %157, %s5.0.lcssa
  %159 = fadd double %158, %s6.0.lcssa
  %160 = fadd double %159, %s7.0.lcssa
  %161 = fadd double %160, %s8.0.lcssa
  store double %161, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB24

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB24:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.MAdd8_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %j.09 = phi i32 [ %153, %bb.nph ], [ 0, %SyncBB ]
  %vectorPHI = phi <16 x double> [ %152, %bb.nph ], [ %8, %SyncBB ]
  %vectorPHI17 = phi <16 x double> [ %150, %bb.nph ], [ %7, %SyncBB ]
  %vectorPHI18 = phi <16 x double> [ %148, %bb.nph ], [ %8, %SyncBB ]
  %vectorPHI19 = phi <16 x double> [ %138, %bb.nph ], [ %7, %SyncBB ]
  %vectorPHI20 = phi <16 x double> [ %140, %bb.nph ], [ %8, %SyncBB ]
  %vectorPHI21 = phi <16 x double> [ %142, %bb.nph ], [ %7, %SyncBB ]
  %vectorPHI22 = phi <16 x double> [ %144, %bb.nph ], [ %8, %SyncBB ]
  %vectorPHI23 = phi <16 x double> [ %146, %bb.nph ], [ %7, %SyncBB ]
  %9 = fmul <16 x double> %vectorPHI19, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %10 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %9
  %11 = fmul <16 x double> %vectorPHI20, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %12 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %11
  %13 = fmul <16 x double> %vectorPHI21, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %14 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %13
  %15 = fmul <16 x double> %vectorPHI22, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %16 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %15
  %17 = fmul <16 x double> %vectorPHI23, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %18 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %17
  %19 = fmul <16 x double> %vectorPHI18, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %20 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %19
  %21 = fmul <16 x double> %vectorPHI17, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %22 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %21
  %23 = fmul <16 x double> %vectorPHI, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %24 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %23
  %25 = fmul <16 x double> %10, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %26 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %25
  %27 = fmul <16 x double> %12, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %28 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %27
  %29 = fmul <16 x double> %14, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %30 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %29
  %31 = fmul <16 x double> %16, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %32 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %31
  %33 = fmul <16 x double> %18, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %34 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %33
  %35 = fmul <16 x double> %20, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %36 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %35
  %37 = fmul <16 x double> %22, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %38 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %37
  %39 = fmul <16 x double> %24, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %40 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %39
  %41 = fmul <16 x double> %26, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %42 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %41
  %43 = fmul <16 x double> %28, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %44 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %43
  %45 = fmul <16 x double> %30, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %46 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %45
  %47 = fmul <16 x double> %32, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %48 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %47
  %49 = fmul <16 x double> %34, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %50 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %49
  %51 = fmul <16 x double> %36, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %52 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %51
  %53 = fmul <16 x double> %38, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %54 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %53
  %55 = fmul <16 x double> %40, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %56 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %55
  %57 = fmul <16 x double> %42, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %58 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %57
  %59 = fmul <16 x double> %44, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %60 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %59
  %61 = fmul <16 x double> %46, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %62 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %61
  %63 = fmul <16 x double> %48, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %64 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %63
  %65 = fmul <16 x double> %50, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %66 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %65
  %67 = fmul <16 x double> %52, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %68 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %67
  %69 = fmul <16 x double> %54, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %70 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %69
  %71 = fmul <16 x double> %56, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %72 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %71
  %73 = fmul <16 x double> %58, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %74 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %73
  %75 = fmul <16 x double> %60, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %76 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %75
  %77 = fmul <16 x double> %62, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %78 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %77
  %79 = fmul <16 x double> %64, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %80 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %79
  %81 = fmul <16 x double> %66, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %82 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %81
  %83 = fmul <16 x double> %68, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %84 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %83
  %85 = fmul <16 x double> %70, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %86 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %85
  %87 = fmul <16 x double> %72, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %88 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %87
  %89 = fmul <16 x double> %74, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %90 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %89
  %91 = fmul <16 x double> %76, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %92 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %91
  %93 = fmul <16 x double> %78, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %94 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %93
  %95 = fmul <16 x double> %80, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %96 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %95
  %97 = fmul <16 x double> %82, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %98 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %97
  %99 = fmul <16 x double> %84, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %100 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %99
  %101 = fmul <16 x double> %86, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %102 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %101
  %103 = fmul <16 x double> %88, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %104 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %103
  %105 = fmul <16 x double> %90, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %106 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %105
  %107 = fmul <16 x double> %92, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %108 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %107
  %109 = fmul <16 x double> %94, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %110 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %109
  %111 = fmul <16 x double> %96, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %112 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %111
  %113 = fmul <16 x double> %98, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %114 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %113
  %115 = fmul <16 x double> %100, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %116 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %115
  %117 = fmul <16 x double> %102, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %118 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %117
  %119 = fmul <16 x double> %104, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %120 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %119
  %121 = fmul <16 x double> %106, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %122 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %121
  %123 = fmul <16 x double> %108, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %124 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %123
  %125 = fmul <16 x double> %110, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %126 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %125
  %127 = fmul <16 x double> %112, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %128 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %127
  %129 = fmul <16 x double> %114, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %130 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %129
  %131 = fmul <16 x double> %116, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %132 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %131
  %133 = fmul <16 x double> %118, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %134 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %133
  %135 = fmul <16 x double> %120, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %136 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %135
  %137 = fmul <16 x double> %122, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %138 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %137
  %139 = fmul <16 x double> %124, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %140 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %139
  %141 = fmul <16 x double> %126, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %142 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %141
  %143 = fmul <16 x double> %128, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %144 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %143
  %145 = fmul <16 x double> %130, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %146 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %145
  %147 = fmul <16 x double> %132, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %148 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %147
  %149 = fmul <16 x double> %134, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %150 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %149
  %151 = fmul <16 x double> %136, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %152 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %151
  %153 = add nsw i32 %j.09, 1
  %exitcond = icmp eq i32 %153, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %vectorPHI24 = phi <16 x double> [ %8, %SyncBB ], [ %152, %bb.nph ]
  %vectorPHI25 = phi <16 x double> [ %7, %SyncBB ], [ %150, %bb.nph ]
  %vectorPHI26 = phi <16 x double> [ %8, %SyncBB ], [ %148, %bb.nph ]
  %vectorPHI27 = phi <16 x double> [ %7, %SyncBB ], [ %138, %bb.nph ]
  %vectorPHI28 = phi <16 x double> [ %8, %SyncBB ], [ %140, %bb.nph ]
  %vectorPHI29 = phi <16 x double> [ %7, %SyncBB ], [ %142, %bb.nph ]
  %vectorPHI30 = phi <16 x double> [ %8, %SyncBB ], [ %144, %bb.nph ]
  %vectorPHI31 = phi <16 x double> [ %7, %SyncBB ], [ %146, %bb.nph ]
  %154 = fadd <16 x double> %vectorPHI27, %vectorPHI28
  %155 = fadd <16 x double> %154, %vectorPHI29
  %156 = fadd <16 x double> %155, %vectorPHI30
  %157 = fadd <16 x double> %156, %vectorPHI31
  %158 = fadd <16 x double> %157, %vectorPHI26
  %159 = fadd <16 x double> %158, %vectorPHI25
  %160 = fadd <16 x double> %159, %vectorPHI24
  %ptrTypeCast32 = bitcast double addrspace(1)* %6 to <16 x double> addrspace(1)*
  store <16 x double> %160, <16 x double> addrspace(1)* %ptrTypeCast32, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB33

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB33:                                         ; preds = %._crit_edge
  ret void
}

define void @MAdd8(i8* %pBuffer) {
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
  %23 = fsub double 1.000000e+01, %22
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %j.09.i = phi i32 [ %168, %bb.nph.i ], [ 0, %SyncBB.i ]
  %s8.08.i = phi double [ %167, %bb.nph.i ], [ %23, %SyncBB.i ]
  %s7.07.i = phi double [ %165, %bb.nph.i ], [ %22, %SyncBB.i ]
  %s6.06.i = phi double [ %163, %bb.nph.i ], [ %23, %SyncBB.i ]
  %s.05.i = phi double [ %153, %bb.nph.i ], [ %22, %SyncBB.i ]
  %s2.04.i = phi double [ %155, %bb.nph.i ], [ %23, %SyncBB.i ]
  %s3.03.i = phi double [ %157, %bb.nph.i ], [ %22, %SyncBB.i ]
  %s4.02.i = phi double [ %159, %bb.nph.i ], [ %23, %SyncBB.i ]
  %s5.01.i = phi double [ %161, %bb.nph.i ], [ %22, %SyncBB.i ]
  %24 = fmul double %s.05.i, 9.899000e-01
  %25 = fsub double 1.000000e+01, %24
  %26 = fmul double %s2.04.i, 9.899000e-01
  %27 = fsub double 1.000000e+01, %26
  %28 = fmul double %s3.03.i, 9.899000e-01
  %29 = fsub double 1.000000e+01, %28
  %30 = fmul double %s4.02.i, 9.899000e-01
  %31 = fsub double 1.000000e+01, %30
  %32 = fmul double %s5.01.i, 9.899000e-01
  %33 = fsub double 1.000000e+01, %32
  %34 = fmul double %s6.06.i, 9.899000e-01
  %35 = fsub double 1.000000e+01, %34
  %36 = fmul double %s7.07.i, 9.899000e-01
  %37 = fsub double 1.000000e+01, %36
  %38 = fmul double %s8.08.i, 9.899000e-01
  %39 = fsub double 1.000000e+01, %38
  %40 = fmul double %25, 9.899000e-01
  %41 = fsub double 1.000000e+01, %40
  %42 = fmul double %27, 9.899000e-01
  %43 = fsub double 1.000000e+01, %42
  %44 = fmul double %29, 9.899000e-01
  %45 = fsub double 1.000000e+01, %44
  %46 = fmul double %31, 9.899000e-01
  %47 = fsub double 1.000000e+01, %46
  %48 = fmul double %33, 9.899000e-01
  %49 = fsub double 1.000000e+01, %48
  %50 = fmul double %35, 9.899000e-01
  %51 = fsub double 1.000000e+01, %50
  %52 = fmul double %37, 9.899000e-01
  %53 = fsub double 1.000000e+01, %52
  %54 = fmul double %39, 9.899000e-01
  %55 = fsub double 1.000000e+01, %54
  %56 = fmul double %41, 9.899000e-01
  %57 = fsub double 1.000000e+01, %56
  %58 = fmul double %43, 9.899000e-01
  %59 = fsub double 1.000000e+01, %58
  %60 = fmul double %45, 9.899000e-01
  %61 = fsub double 1.000000e+01, %60
  %62 = fmul double %47, 9.899000e-01
  %63 = fsub double 1.000000e+01, %62
  %64 = fmul double %49, 9.899000e-01
  %65 = fsub double 1.000000e+01, %64
  %66 = fmul double %51, 9.899000e-01
  %67 = fsub double 1.000000e+01, %66
  %68 = fmul double %53, 9.899000e-01
  %69 = fsub double 1.000000e+01, %68
  %70 = fmul double %55, 9.899000e-01
  %71 = fsub double 1.000000e+01, %70
  %72 = fmul double %57, 9.899000e-01
  %73 = fsub double 1.000000e+01, %72
  %74 = fmul double %59, 9.899000e-01
  %75 = fsub double 1.000000e+01, %74
  %76 = fmul double %61, 9.899000e-01
  %77 = fsub double 1.000000e+01, %76
  %78 = fmul double %63, 9.899000e-01
  %79 = fsub double 1.000000e+01, %78
  %80 = fmul double %65, 9.899000e-01
  %81 = fsub double 1.000000e+01, %80
  %82 = fmul double %67, 9.899000e-01
  %83 = fsub double 1.000000e+01, %82
  %84 = fmul double %69, 9.899000e-01
  %85 = fsub double 1.000000e+01, %84
  %86 = fmul double %71, 9.899000e-01
  %87 = fsub double 1.000000e+01, %86
  %88 = fmul double %73, 9.899000e-01
  %89 = fsub double 1.000000e+01, %88
  %90 = fmul double %75, 9.899000e-01
  %91 = fsub double 1.000000e+01, %90
  %92 = fmul double %77, 9.899000e-01
  %93 = fsub double 1.000000e+01, %92
  %94 = fmul double %79, 9.899000e-01
  %95 = fsub double 1.000000e+01, %94
  %96 = fmul double %81, 9.899000e-01
  %97 = fsub double 1.000000e+01, %96
  %98 = fmul double %83, 9.899000e-01
  %99 = fsub double 1.000000e+01, %98
  %100 = fmul double %85, 9.899000e-01
  %101 = fsub double 1.000000e+01, %100
  %102 = fmul double %87, 9.899000e-01
  %103 = fsub double 1.000000e+01, %102
  %104 = fmul double %89, 9.899000e-01
  %105 = fsub double 1.000000e+01, %104
  %106 = fmul double %91, 9.899000e-01
  %107 = fsub double 1.000000e+01, %106
  %108 = fmul double %93, 9.899000e-01
  %109 = fsub double 1.000000e+01, %108
  %110 = fmul double %95, 9.899000e-01
  %111 = fsub double 1.000000e+01, %110
  %112 = fmul double %97, 9.899000e-01
  %113 = fsub double 1.000000e+01, %112
  %114 = fmul double %99, 9.899000e-01
  %115 = fsub double 1.000000e+01, %114
  %116 = fmul double %101, 9.899000e-01
  %117 = fsub double 1.000000e+01, %116
  %118 = fmul double %103, 9.899000e-01
  %119 = fsub double 1.000000e+01, %118
  %120 = fmul double %105, 9.899000e-01
  %121 = fsub double 1.000000e+01, %120
  %122 = fmul double %107, 9.899000e-01
  %123 = fsub double 1.000000e+01, %122
  %124 = fmul double %109, 9.899000e-01
  %125 = fsub double 1.000000e+01, %124
  %126 = fmul double %111, 9.899000e-01
  %127 = fsub double 1.000000e+01, %126
  %128 = fmul double %113, 9.899000e-01
  %129 = fsub double 1.000000e+01, %128
  %130 = fmul double %115, 9.899000e-01
  %131 = fsub double 1.000000e+01, %130
  %132 = fmul double %117, 9.899000e-01
  %133 = fsub double 1.000000e+01, %132
  %134 = fmul double %119, 9.899000e-01
  %135 = fsub double 1.000000e+01, %134
  %136 = fmul double %121, 9.899000e-01
  %137 = fsub double 1.000000e+01, %136
  %138 = fmul double %123, 9.899000e-01
  %139 = fsub double 1.000000e+01, %138
  %140 = fmul double %125, 9.899000e-01
  %141 = fsub double 1.000000e+01, %140
  %142 = fmul double %127, 9.899000e-01
  %143 = fsub double 1.000000e+01, %142
  %144 = fmul double %129, 9.899000e-01
  %145 = fsub double 1.000000e+01, %144
  %146 = fmul double %131, 9.899000e-01
  %147 = fsub double 1.000000e+01, %146
  %148 = fmul double %133, 9.899000e-01
  %149 = fsub double 1.000000e+01, %148
  %150 = fmul double %135, 9.899000e-01
  %151 = fsub double 1.000000e+01, %150
  %152 = fmul double %137, 9.899000e-01
  %153 = fsub double 1.000000e+01, %152
  %154 = fmul double %139, 9.899000e-01
  %155 = fsub double 1.000000e+01, %154
  %156 = fmul double %141, 9.899000e-01
  %157 = fsub double 1.000000e+01, %156
  %158 = fmul double %143, 9.899000e-01
  %159 = fsub double 1.000000e+01, %158
  %160 = fmul double %145, 9.899000e-01
  %161 = fsub double 1.000000e+01, %160
  %162 = fmul double %147, 9.899000e-01
  %163 = fsub double 1.000000e+01, %162
  %164 = fmul double %149, 9.899000e-01
  %165 = fsub double 1.000000e+01, %164
  %166 = fmul double %151, 9.899000e-01
  %167 = fsub double 1.000000e+01, %166
  %168 = add nsw i32 %j.09.i, 1
  %exitcond.i = icmp eq i32 %168, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %s8.0.lcssa.i = phi double [ %23, %SyncBB.i ], [ %167, %bb.nph.i ]
  %s7.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %165, %bb.nph.i ]
  %s6.0.lcssa.i = phi double [ %23, %SyncBB.i ], [ %163, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %153, %bb.nph.i ]
  %s2.0.lcssa.i = phi double [ %23, %SyncBB.i ], [ %155, %bb.nph.i ]
  %s3.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %157, %bb.nph.i ]
  %s4.0.lcssa.i = phi double [ %23, %SyncBB.i ], [ %159, %bb.nph.i ]
  %s5.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %161, %bb.nph.i ]
  %169 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  %170 = fadd double %169, %s3.0.lcssa.i
  %171 = fadd double %170, %s4.0.lcssa.i
  %172 = fadd double %171, %s5.0.lcssa.i
  %173 = fadd double %172, %s6.0.lcssa.i
  %174 = fadd double %173, %s7.0.lcssa.i
  %175 = fadd double %174, %s8.0.lcssa.i
  store double %175, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MAdd8_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__MAdd8_separated_args.exit:                      ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.MAdd8(i8* %pBuffer) {
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
  %j.09.i = phi i32 [ %167, %bb.nph.i ], [ 0, %SyncBB.i ]
  %vectorPHI.i = phi <16 x double> [ %166, %bb.nph.i ], [ %22, %SyncBB.i ]
  %vectorPHI17.i = phi <16 x double> [ %164, %bb.nph.i ], [ %21, %SyncBB.i ]
  %vectorPHI18.i = phi <16 x double> [ %162, %bb.nph.i ], [ %22, %SyncBB.i ]
  %vectorPHI19.i = phi <16 x double> [ %152, %bb.nph.i ], [ %21, %SyncBB.i ]
  %vectorPHI20.i = phi <16 x double> [ %154, %bb.nph.i ], [ %22, %SyncBB.i ]
  %vectorPHI21.i = phi <16 x double> [ %156, %bb.nph.i ], [ %21, %SyncBB.i ]
  %vectorPHI22.i = phi <16 x double> [ %158, %bb.nph.i ], [ %22, %SyncBB.i ]
  %vectorPHI23.i = phi <16 x double> [ %160, %bb.nph.i ], [ %21, %SyncBB.i ]
  %23 = fmul <16 x double> %vectorPHI19.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %24 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %23
  %25 = fmul <16 x double> %vectorPHI20.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %26 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %25
  %27 = fmul <16 x double> %vectorPHI21.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %28 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %27
  %29 = fmul <16 x double> %vectorPHI22.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %30 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %29
  %31 = fmul <16 x double> %vectorPHI23.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %32 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %31
  %33 = fmul <16 x double> %vectorPHI18.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %34 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %33
  %35 = fmul <16 x double> %vectorPHI17.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %36 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %35
  %37 = fmul <16 x double> %vectorPHI.i, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %38 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %37
  %39 = fmul <16 x double> %24, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %40 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %39
  %41 = fmul <16 x double> %26, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %42 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %41
  %43 = fmul <16 x double> %28, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %44 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %43
  %45 = fmul <16 x double> %30, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %46 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %45
  %47 = fmul <16 x double> %32, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %48 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %47
  %49 = fmul <16 x double> %34, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %50 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %49
  %51 = fmul <16 x double> %36, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %52 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %51
  %53 = fmul <16 x double> %38, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %54 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %53
  %55 = fmul <16 x double> %40, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %56 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %55
  %57 = fmul <16 x double> %42, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %58 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %57
  %59 = fmul <16 x double> %44, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %60 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %59
  %61 = fmul <16 x double> %46, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %62 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %61
  %63 = fmul <16 x double> %48, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %64 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %63
  %65 = fmul <16 x double> %50, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %66 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %65
  %67 = fmul <16 x double> %52, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %68 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %67
  %69 = fmul <16 x double> %54, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %70 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %69
  %71 = fmul <16 x double> %56, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %72 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %71
  %73 = fmul <16 x double> %58, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %74 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %73
  %75 = fmul <16 x double> %60, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %76 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %75
  %77 = fmul <16 x double> %62, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %78 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %77
  %79 = fmul <16 x double> %64, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %80 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %79
  %81 = fmul <16 x double> %66, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %82 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %81
  %83 = fmul <16 x double> %68, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %84 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %83
  %85 = fmul <16 x double> %70, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %86 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %85
  %87 = fmul <16 x double> %72, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %88 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %87
  %89 = fmul <16 x double> %74, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %90 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %89
  %91 = fmul <16 x double> %76, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %92 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %91
  %93 = fmul <16 x double> %78, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %94 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %93
  %95 = fmul <16 x double> %80, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %96 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %95
  %97 = fmul <16 x double> %82, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %98 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %97
  %99 = fmul <16 x double> %84, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %100 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %99
  %101 = fmul <16 x double> %86, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %102 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %101
  %103 = fmul <16 x double> %88, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %104 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %103
  %105 = fmul <16 x double> %90, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %106 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %105
  %107 = fmul <16 x double> %92, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %108 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %107
  %109 = fmul <16 x double> %94, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %110 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %109
  %111 = fmul <16 x double> %96, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %112 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %111
  %113 = fmul <16 x double> %98, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %114 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %113
  %115 = fmul <16 x double> %100, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %116 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %115
  %117 = fmul <16 x double> %102, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %118 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %117
  %119 = fmul <16 x double> %104, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %120 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %119
  %121 = fmul <16 x double> %106, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %122 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %121
  %123 = fmul <16 x double> %108, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %124 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %123
  %125 = fmul <16 x double> %110, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %126 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %125
  %127 = fmul <16 x double> %112, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %128 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %127
  %129 = fmul <16 x double> %114, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %130 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %129
  %131 = fmul <16 x double> %116, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %132 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %131
  %133 = fmul <16 x double> %118, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %134 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %133
  %135 = fmul <16 x double> %120, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %136 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %135
  %137 = fmul <16 x double> %122, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %138 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %137
  %139 = fmul <16 x double> %124, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %140 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %139
  %141 = fmul <16 x double> %126, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %142 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %141
  %143 = fmul <16 x double> %128, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %144 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %143
  %145 = fmul <16 x double> %130, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %146 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %145
  %147 = fmul <16 x double> %132, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %148 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %147
  %149 = fmul <16 x double> %134, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %150 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %149
  %151 = fmul <16 x double> %136, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %152 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %151
  %153 = fmul <16 x double> %138, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %154 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %153
  %155 = fmul <16 x double> %140, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %156 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %155
  %157 = fmul <16 x double> %142, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %158 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %157
  %159 = fmul <16 x double> %144, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %160 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %159
  %161 = fmul <16 x double> %146, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %162 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %161
  %163 = fmul <16 x double> %148, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %164 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %163
  %165 = fmul <16 x double> %150, <double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01, double 9.899000e-01>
  %166 = fsub <16 x double> <double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01, double 1.000000e+01>, %165
  %167 = add nsw i32 %j.09.i, 1
  %exitcond.i = icmp eq i32 %167, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %vectorPHI24.i = phi <16 x double> [ %22, %SyncBB.i ], [ %166, %bb.nph.i ]
  %vectorPHI25.i = phi <16 x double> [ %21, %SyncBB.i ], [ %164, %bb.nph.i ]
  %vectorPHI26.i = phi <16 x double> [ %22, %SyncBB.i ], [ %162, %bb.nph.i ]
  %vectorPHI27.i = phi <16 x double> [ %21, %SyncBB.i ], [ %152, %bb.nph.i ]
  %vectorPHI28.i = phi <16 x double> [ %22, %SyncBB.i ], [ %154, %bb.nph.i ]
  %vectorPHI29.i = phi <16 x double> [ %21, %SyncBB.i ], [ %156, %bb.nph.i ]
  %vectorPHI30.i = phi <16 x double> [ %22, %SyncBB.i ], [ %158, %bb.nph.i ]
  %vectorPHI31.i = phi <16 x double> [ %21, %SyncBB.i ], [ %160, %bb.nph.i ]
  %168 = fadd <16 x double> %vectorPHI27.i, %vectorPHI28.i
  %169 = fadd <16 x double> %168, %vectorPHI29.i
  %170 = fadd <16 x double> %169, %vectorPHI30.i
  %171 = fadd <16 x double> %170, %vectorPHI31.i
  %172 = fadd <16 x double> %171, %vectorPHI26.i
  %173 = fadd <16 x double> %172, %vectorPHI25.i
  %174 = fadd <16 x double> %173, %vectorPHI24.i
  %ptrTypeCast32.i = bitcast double addrspace(1)* %20 to <16 x double> addrspace(1)*
  store <16 x double> %174, <16 x double> addrspace(1)* %ptrTypeCast32.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.MAdd8_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.MAdd8_separated_args.exit:        ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MAdd8_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MAdd8_locals_anchor", void (i8*)* @MAdd8}
!1 = metadata !{i32 0, i32 0, i32 0}
