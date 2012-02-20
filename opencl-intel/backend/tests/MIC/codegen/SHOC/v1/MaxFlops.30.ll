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

declare void @__MAdd4_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__MAdd4_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB12

SyncBB12:                                         ; preds = %thenBB, %FirstBB
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

bb.nph:                                           ; preds = %SyncBB12, %bb.nph
  %j.05 = phi i32 [ %146, %bb.nph ], [ 0, %SyncBB12 ]
  %s4.04 = phi double [ %145, %bb.nph ], [ %9, %SyncBB12 ]
  %s3.03 = phi double [ %143, %bb.nph ], [ %8, %SyncBB12 ]
  %s2.02 = phi double [ %141, %bb.nph ], [ %9, %SyncBB12 ]
  %s.01 = phi double [ %139, %bb.nph ], [ %8, %SyncBB12 ]
  %10 = fmul double %s.01, 9.899000e-01
  %11 = fsub double 1.000000e+01, %10
  %12 = fmul double %s2.02, 9.899000e-01
  %13 = fsub double 1.000000e+01, %12
  %14 = fmul double %s3.03, 9.899000e-01
  %15 = fsub double 1.000000e+01, %14
  %16 = fmul double %s4.04, 9.899000e-01
  %17 = fsub double 1.000000e+01, %16
  %18 = fmul double %11, 9.899000e-01
  %19 = fsub double 1.000000e+01, %18
  %20 = fmul double %13, 9.899000e-01
  %21 = fsub double 1.000000e+01, %20
  %22 = fmul double %15, 9.899000e-01
  %23 = fsub double 1.000000e+01, %22
  %24 = fmul double %17, 9.899000e-01
  %25 = fsub double 1.000000e+01, %24
  %26 = fmul double %19, 9.899000e-01
  %27 = fsub double 1.000000e+01, %26
  %28 = fmul double %21, 9.899000e-01
  %29 = fsub double 1.000000e+01, %28
  %30 = fmul double %23, 9.899000e-01
  %31 = fsub double 1.000000e+01, %30
  %32 = fmul double %25, 9.899000e-01
  %33 = fsub double 1.000000e+01, %32
  %34 = fmul double %27, 9.899000e-01
  %35 = fsub double 1.000000e+01, %34
  %36 = fmul double %29, 9.899000e-01
  %37 = fsub double 1.000000e+01, %36
  %38 = fmul double %31, 9.899000e-01
  %39 = fsub double 1.000000e+01, %38
  %40 = fmul double %33, 9.899000e-01
  %41 = fsub double 1.000000e+01, %40
  %42 = fmul double %35, 9.899000e-01
  %43 = fsub double 1.000000e+01, %42
  %44 = fmul double %37, 9.899000e-01
  %45 = fsub double 1.000000e+01, %44
  %46 = fmul double %39, 9.899000e-01
  %47 = fsub double 1.000000e+01, %46
  %48 = fmul double %41, 9.899000e-01
  %49 = fsub double 1.000000e+01, %48
  %50 = fmul double %43, 9.899000e-01
  %51 = fsub double 1.000000e+01, %50
  %52 = fmul double %45, 9.899000e-01
  %53 = fsub double 1.000000e+01, %52
  %54 = fmul double %47, 9.899000e-01
  %55 = fsub double 1.000000e+01, %54
  %56 = fmul double %49, 9.899000e-01
  %57 = fsub double 1.000000e+01, %56
  %58 = fmul double %51, 9.899000e-01
  %59 = fsub double 1.000000e+01, %58
  %60 = fmul double %53, 9.899000e-01
  %61 = fsub double 1.000000e+01, %60
  %62 = fmul double %55, 9.899000e-01
  %63 = fsub double 1.000000e+01, %62
  %64 = fmul double %57, 9.899000e-01
  %65 = fsub double 1.000000e+01, %64
  %66 = fmul double %59, 9.899000e-01
  %67 = fsub double 1.000000e+01, %66
  %68 = fmul double %61, 9.899000e-01
  %69 = fsub double 1.000000e+01, %68
  %70 = fmul double %63, 9.899000e-01
  %71 = fsub double 1.000000e+01, %70
  %72 = fmul double %65, 9.899000e-01
  %73 = fsub double 1.000000e+01, %72
  %74 = fmul double %67, 9.899000e-01
  %75 = fsub double 1.000000e+01, %74
  %76 = fmul double %69, 9.899000e-01
  %77 = fsub double 1.000000e+01, %76
  %78 = fmul double %71, 9.899000e-01
  %79 = fsub double 1.000000e+01, %78
  %80 = fmul double %73, 9.899000e-01
  %81 = fsub double 1.000000e+01, %80
  %82 = fmul double %75, 9.899000e-01
  %83 = fsub double 1.000000e+01, %82
  %84 = fmul double %77, 9.899000e-01
  %85 = fsub double 1.000000e+01, %84
  %86 = fmul double %79, 9.899000e-01
  %87 = fsub double 1.000000e+01, %86
  %88 = fmul double %81, 9.899000e-01
  %89 = fsub double 1.000000e+01, %88
  %90 = fmul double %83, 9.899000e-01
  %91 = fsub double 1.000000e+01, %90
  %92 = fmul double %85, 9.899000e-01
  %93 = fsub double 1.000000e+01, %92
  %94 = fmul double %87, 9.899000e-01
  %95 = fsub double 1.000000e+01, %94
  %96 = fmul double %89, 9.899000e-01
  %97 = fsub double 1.000000e+01, %96
  %98 = fmul double %91, 9.899000e-01
  %99 = fsub double 1.000000e+01, %98
  %100 = fmul double %93, 9.899000e-01
  %101 = fsub double 1.000000e+01, %100
  %102 = fmul double %95, 9.899000e-01
  %103 = fsub double 1.000000e+01, %102
  %104 = fmul double %97, 9.899000e-01
  %105 = fsub double 1.000000e+01, %104
  %106 = fmul double %99, 9.899000e-01
  %107 = fsub double 1.000000e+01, %106
  %108 = fmul double %101, 9.899000e-01
  %109 = fsub double 1.000000e+01, %108
  %110 = fmul double %103, 9.899000e-01
  %111 = fsub double 1.000000e+01, %110
  %112 = fmul double %105, 9.899000e-01
  %113 = fsub double 1.000000e+01, %112
  %114 = fmul double %107, 9.899000e-01
  %115 = fsub double 1.000000e+01, %114
  %116 = fmul double %109, 9.899000e-01
  %117 = fsub double 1.000000e+01, %116
  %118 = fmul double %111, 9.899000e-01
  %119 = fsub double 1.000000e+01, %118
  %120 = fmul double %113, 9.899000e-01
  %121 = fsub double 1.000000e+01, %120
  %122 = fmul double %115, 9.899000e-01
  %123 = fsub double 1.000000e+01, %122
  %124 = fmul double %117, 9.899000e-01
  %125 = fsub double 1.000000e+01, %124
  %126 = fmul double %119, 9.899000e-01
  %127 = fsub double 1.000000e+01, %126
  %128 = fmul double %121, 9.899000e-01
  %129 = fsub double 1.000000e+01, %128
  %130 = fmul double %123, 9.899000e-01
  %131 = fsub double 1.000000e+01, %130
  %132 = fmul double %125, 9.899000e-01
  %133 = fsub double 1.000000e+01, %132
  %134 = fmul double %127, 9.899000e-01
  %135 = fsub double 1.000000e+01, %134
  %136 = fmul double %129, 9.899000e-01
  %137 = fsub double 1.000000e+01, %136
  %138 = fmul double %131, 9.899000e-01
  %139 = fsub double 1.000000e+01, %138
  %140 = fmul double %133, 9.899000e-01
  %141 = fsub double 1.000000e+01, %140
  %142 = fmul double %135, 9.899000e-01
  %143 = fsub double 1.000000e+01, %142
  %144 = fmul double %137, 9.899000e-01
  %145 = fsub double 1.000000e+01, %144
  %146 = add nsw i32 %j.05, 1
  %exitcond = icmp eq i32 %146, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB12
  %s4.0.lcssa = phi double [ %9, %SyncBB12 ], [ %145, %bb.nph ]
  %s3.0.lcssa = phi double [ %8, %SyncBB12 ], [ %143, %bb.nph ]
  %s2.0.lcssa = phi double [ %9, %SyncBB12 ], [ %141, %bb.nph ]
  %s.0.lcssa = phi double [ %8, %SyncBB12 ], [ %139, %bb.nph ]
  %147 = fadd double %s.0.lcssa, %s2.0.lcssa
  %148 = fadd double %147, %s3.0.lcssa
  %149 = fadd double %148, %s4.0.lcssa
  store double %149, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB12

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @MAdd4(i8* %pBuffer) {
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
  br label %SyncBB12.i

SyncBB12.i:                                       ; preds = %thenBB.i, %entry
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

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB12.i
  %j.05.i = phi i32 [ %160, %bb.nph.i ], [ 0, %SyncBB12.i ]
  %s4.04.i = phi double [ %159, %bb.nph.i ], [ %23, %SyncBB12.i ]
  %s3.03.i = phi double [ %157, %bb.nph.i ], [ %22, %SyncBB12.i ]
  %s2.02.i = phi double [ %155, %bb.nph.i ], [ %23, %SyncBB12.i ]
  %s.01.i = phi double [ %153, %bb.nph.i ], [ %22, %SyncBB12.i ]
  %24 = fmul double %s.01.i, 9.899000e-01
  %25 = fsub double 1.000000e+01, %24
  %26 = fmul double %s2.02.i, 9.899000e-01
  %27 = fsub double 1.000000e+01, %26
  %28 = fmul double %s3.03.i, 9.899000e-01
  %29 = fsub double 1.000000e+01, %28
  %30 = fmul double %s4.04.i, 9.899000e-01
  %31 = fsub double 1.000000e+01, %30
  %32 = fmul double %25, 9.899000e-01
  %33 = fsub double 1.000000e+01, %32
  %34 = fmul double %27, 9.899000e-01
  %35 = fsub double 1.000000e+01, %34
  %36 = fmul double %29, 9.899000e-01
  %37 = fsub double 1.000000e+01, %36
  %38 = fmul double %31, 9.899000e-01
  %39 = fsub double 1.000000e+01, %38
  %40 = fmul double %33, 9.899000e-01
  %41 = fsub double 1.000000e+01, %40
  %42 = fmul double %35, 9.899000e-01
  %43 = fsub double 1.000000e+01, %42
  %44 = fmul double %37, 9.899000e-01
  %45 = fsub double 1.000000e+01, %44
  %46 = fmul double %39, 9.899000e-01
  %47 = fsub double 1.000000e+01, %46
  %48 = fmul double %41, 9.899000e-01
  %49 = fsub double 1.000000e+01, %48
  %50 = fmul double %43, 9.899000e-01
  %51 = fsub double 1.000000e+01, %50
  %52 = fmul double %45, 9.899000e-01
  %53 = fsub double 1.000000e+01, %52
  %54 = fmul double %47, 9.899000e-01
  %55 = fsub double 1.000000e+01, %54
  %56 = fmul double %49, 9.899000e-01
  %57 = fsub double 1.000000e+01, %56
  %58 = fmul double %51, 9.899000e-01
  %59 = fsub double 1.000000e+01, %58
  %60 = fmul double %53, 9.899000e-01
  %61 = fsub double 1.000000e+01, %60
  %62 = fmul double %55, 9.899000e-01
  %63 = fsub double 1.000000e+01, %62
  %64 = fmul double %57, 9.899000e-01
  %65 = fsub double 1.000000e+01, %64
  %66 = fmul double %59, 9.899000e-01
  %67 = fsub double 1.000000e+01, %66
  %68 = fmul double %61, 9.899000e-01
  %69 = fsub double 1.000000e+01, %68
  %70 = fmul double %63, 9.899000e-01
  %71 = fsub double 1.000000e+01, %70
  %72 = fmul double %65, 9.899000e-01
  %73 = fsub double 1.000000e+01, %72
  %74 = fmul double %67, 9.899000e-01
  %75 = fsub double 1.000000e+01, %74
  %76 = fmul double %69, 9.899000e-01
  %77 = fsub double 1.000000e+01, %76
  %78 = fmul double %71, 9.899000e-01
  %79 = fsub double 1.000000e+01, %78
  %80 = fmul double %73, 9.899000e-01
  %81 = fsub double 1.000000e+01, %80
  %82 = fmul double %75, 9.899000e-01
  %83 = fsub double 1.000000e+01, %82
  %84 = fmul double %77, 9.899000e-01
  %85 = fsub double 1.000000e+01, %84
  %86 = fmul double %79, 9.899000e-01
  %87 = fsub double 1.000000e+01, %86
  %88 = fmul double %81, 9.899000e-01
  %89 = fsub double 1.000000e+01, %88
  %90 = fmul double %83, 9.899000e-01
  %91 = fsub double 1.000000e+01, %90
  %92 = fmul double %85, 9.899000e-01
  %93 = fsub double 1.000000e+01, %92
  %94 = fmul double %87, 9.899000e-01
  %95 = fsub double 1.000000e+01, %94
  %96 = fmul double %89, 9.899000e-01
  %97 = fsub double 1.000000e+01, %96
  %98 = fmul double %91, 9.899000e-01
  %99 = fsub double 1.000000e+01, %98
  %100 = fmul double %93, 9.899000e-01
  %101 = fsub double 1.000000e+01, %100
  %102 = fmul double %95, 9.899000e-01
  %103 = fsub double 1.000000e+01, %102
  %104 = fmul double %97, 9.899000e-01
  %105 = fsub double 1.000000e+01, %104
  %106 = fmul double %99, 9.899000e-01
  %107 = fsub double 1.000000e+01, %106
  %108 = fmul double %101, 9.899000e-01
  %109 = fsub double 1.000000e+01, %108
  %110 = fmul double %103, 9.899000e-01
  %111 = fsub double 1.000000e+01, %110
  %112 = fmul double %105, 9.899000e-01
  %113 = fsub double 1.000000e+01, %112
  %114 = fmul double %107, 9.899000e-01
  %115 = fsub double 1.000000e+01, %114
  %116 = fmul double %109, 9.899000e-01
  %117 = fsub double 1.000000e+01, %116
  %118 = fmul double %111, 9.899000e-01
  %119 = fsub double 1.000000e+01, %118
  %120 = fmul double %113, 9.899000e-01
  %121 = fsub double 1.000000e+01, %120
  %122 = fmul double %115, 9.899000e-01
  %123 = fsub double 1.000000e+01, %122
  %124 = fmul double %117, 9.899000e-01
  %125 = fsub double 1.000000e+01, %124
  %126 = fmul double %119, 9.899000e-01
  %127 = fsub double 1.000000e+01, %126
  %128 = fmul double %121, 9.899000e-01
  %129 = fsub double 1.000000e+01, %128
  %130 = fmul double %123, 9.899000e-01
  %131 = fsub double 1.000000e+01, %130
  %132 = fmul double %125, 9.899000e-01
  %133 = fsub double 1.000000e+01, %132
  %134 = fmul double %127, 9.899000e-01
  %135 = fsub double 1.000000e+01, %134
  %136 = fmul double %129, 9.899000e-01
  %137 = fsub double 1.000000e+01, %136
  %138 = fmul double %131, 9.899000e-01
  %139 = fsub double 1.000000e+01, %138
  %140 = fmul double %133, 9.899000e-01
  %141 = fsub double 1.000000e+01, %140
  %142 = fmul double %135, 9.899000e-01
  %143 = fsub double 1.000000e+01, %142
  %144 = fmul double %137, 9.899000e-01
  %145 = fsub double 1.000000e+01, %144
  %146 = fmul double %139, 9.899000e-01
  %147 = fsub double 1.000000e+01, %146
  %148 = fmul double %141, 9.899000e-01
  %149 = fsub double 1.000000e+01, %148
  %150 = fmul double %143, 9.899000e-01
  %151 = fsub double 1.000000e+01, %150
  %152 = fmul double %145, 9.899000e-01
  %153 = fsub double 1.000000e+01, %152
  %154 = fmul double %147, 9.899000e-01
  %155 = fsub double 1.000000e+01, %154
  %156 = fmul double %149, 9.899000e-01
  %157 = fsub double 1.000000e+01, %156
  %158 = fmul double %151, 9.899000e-01
  %159 = fsub double 1.000000e+01, %158
  %160 = add nsw i32 %j.05.i, 1
  %exitcond.i = icmp eq i32 %160, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB12.i
  %s4.0.lcssa.i = phi double [ %23, %SyncBB12.i ], [ %159, %bb.nph.i ]
  %s3.0.lcssa.i = phi double [ %22, %SyncBB12.i ], [ %157, %bb.nph.i ]
  %s2.0.lcssa.i = phi double [ %23, %SyncBB12.i ], [ %155, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %22, %SyncBB12.i ], [ %153, %bb.nph.i ]
  %161 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  %162 = fadd double %161, %s3.0.lcssa.i
  %163 = fadd double %162, %s4.0.lcssa.i
  store double %163, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__MAdd4_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB12.i

__MAdd4_separated_args.exit:                      ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__MAdd4_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_MAdd4_locals_anchor", void (i8*)* @MAdd4}
!1 = metadata !{i32 0, i32 0, i32 0}
