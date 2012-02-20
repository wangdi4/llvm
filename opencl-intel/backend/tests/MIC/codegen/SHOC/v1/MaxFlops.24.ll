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

declare void @__Mul1_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__Mul1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %9 = fsub double %8, %8
  %10 = fadd double %9, 9.990000e-01
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB3, %bb.nph
  %j.02 = phi i32 [ %127, %bb.nph ], [ 0, %SyncBB3 ]
  %s.01 = phi double [ %126, %bb.nph ], [ %10, %SyncBB3 ]
  %11 = fmul double %s.01, %s.01
  %12 = fmul double %11, 1.010000e+00
  %13 = fmul double %12, %12
  %14 = fmul double %13, 1.010000e+00
  %15 = fmul double %14, %14
  %16 = fmul double %15, 1.010000e+00
  %17 = fmul double %16, %16
  %18 = fmul double %17, 1.010000e+00
  %19 = fmul double %18, %18
  %20 = fmul double %19, 1.010000e+00
  %21 = fmul double %20, %20
  %22 = fmul double %21, 1.010000e+00
  %23 = fmul double %22, %22
  %24 = fmul double %23, 1.010000e+00
  %25 = fmul double %24, %24
  %26 = fmul double %25, 1.010000e+00
  %27 = fmul double %26, %26
  %28 = fmul double %27, 1.010000e+00
  %29 = fmul double %28, %28
  %30 = fmul double %29, 1.010000e+00
  %31 = fmul double %30, %30
  %32 = fmul double %31, 1.010000e+00
  %33 = fmul double %32, %32
  %34 = fmul double %33, 1.010000e+00
  %35 = fmul double %34, %34
  %36 = fmul double %35, 1.010000e+00
  %37 = fmul double %36, %36
  %38 = fmul double %37, 1.010000e+00
  %39 = fmul double %38, %38
  %40 = fmul double %39, 1.010000e+00
  %41 = fmul double %40, %40
  %42 = fmul double %41, 1.010000e+00
  %43 = fmul double %42, %42
  %44 = fmul double %43, 1.010000e+00
  %45 = fmul double %44, %44
  %46 = fmul double %45, 1.010000e+00
  %47 = fmul double %46, %46
  %48 = fmul double %47, 1.010000e+00
  %49 = fmul double %48, %48
  %50 = fmul double %49, 1.010000e+00
  %51 = fmul double %50, %50
  %52 = fmul double %51, 1.010000e+00
  %53 = fmul double %52, %52
  %54 = fmul double %53, 1.010000e+00
  %55 = fmul double %54, %54
  %56 = fmul double %55, 1.010000e+00
  %57 = fmul double %56, %56
  %58 = fmul double %57, 1.010000e+00
  %59 = fmul double %58, %58
  %60 = fmul double %59, 1.010000e+00
  %61 = fmul double %60, %60
  %62 = fmul double %61, 1.010000e+00
  %63 = fmul double %62, %62
  %64 = fmul double %63, 1.010000e+00
  %65 = fmul double %64, %64
  %66 = fmul double %65, 1.010000e+00
  %67 = fmul double %66, %66
  %68 = fmul double %67, 1.010000e+00
  %69 = fmul double %68, %68
  %70 = fmul double %69, 1.010000e+00
  %71 = fmul double %70, %70
  %72 = fmul double %71, 1.010000e+00
  %73 = fmul double %72, %72
  %74 = fmul double %73, 1.010000e+00
  %75 = fmul double %74, %74
  %76 = fmul double %75, 1.010000e+00
  %77 = fmul double %76, %76
  %78 = fmul double %77, 1.010000e+00
  %79 = fmul double %78, %78
  %80 = fmul double %79, 1.010000e+00
  %81 = fmul double %80, %80
  %82 = fmul double %81, 1.010000e+00
  %83 = fmul double %82, %82
  %84 = fmul double %83, 1.010000e+00
  %85 = fmul double %84, %84
  %86 = fmul double %85, 1.010000e+00
  %87 = fmul double %86, %86
  %88 = fmul double %87, 1.010000e+00
  %89 = fmul double %88, %88
  %90 = fmul double %89, 1.010000e+00
  %91 = fmul double %90, %90
  %92 = fmul double %91, 1.010000e+00
  %93 = fmul double %92, %92
  %94 = fmul double %93, 1.010000e+00
  %95 = fmul double %94, %94
  %96 = fmul double %95, 1.010000e+00
  %97 = fmul double %96, %96
  %98 = fmul double %97, 1.010000e+00
  %99 = fmul double %98, %98
  %100 = fmul double %99, 1.010000e+00
  %101 = fmul double %100, %100
  %102 = fmul double %101, 1.010000e+00
  %103 = fmul double %102, %102
  %104 = fmul double %103, 1.010000e+00
  %105 = fmul double %104, %104
  %106 = fmul double %105, 1.010000e+00
  %107 = fmul double %106, %106
  %108 = fmul double %107, 1.010000e+00
  %109 = fmul double %108, %108
  %110 = fmul double %109, 1.010000e+00
  %111 = fmul double %110, %110
  %112 = fmul double %111, 1.010000e+00
  %113 = fmul double %112, %112
  %114 = fmul double %113, 1.010000e+00
  %115 = fmul double %114, %114
  %116 = fmul double %115, 1.010000e+00
  %117 = fmul double %116, %116
  %118 = fmul double %117, 1.010000e+00
  %119 = fmul double %118, %118
  %120 = fmul double %119, 1.010000e+00
  %121 = fmul double %120, %120
  %122 = fmul double %121, 1.010000e+00
  %123 = fmul double %122, %122
  %124 = fmul double %123, 1.010000e+00
  %125 = fmul double %124, %124
  %126 = fmul double %125, 1.010000e+00
  %127 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %127, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB3
  %s.0.lcssa = phi double [ %10, %SyncBB3 ], [ %126, %bb.nph ]
  store double %s.0.lcssa, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @Mul1(i8* %pBuffer) {
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
  %23 = fsub double %22, %22
  %24 = fadd double %23, 9.990000e-01
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB3.i
  %j.02.i = phi i32 [ %141, %bb.nph.i ], [ 0, %SyncBB3.i ]
  %s.01.i = phi double [ %140, %bb.nph.i ], [ %24, %SyncBB3.i ]
  %25 = fmul double %s.01.i, %s.01.i
  %26 = fmul double %25, 1.010000e+00
  %27 = fmul double %26, %26
  %28 = fmul double %27, 1.010000e+00
  %29 = fmul double %28, %28
  %30 = fmul double %29, 1.010000e+00
  %31 = fmul double %30, %30
  %32 = fmul double %31, 1.010000e+00
  %33 = fmul double %32, %32
  %34 = fmul double %33, 1.010000e+00
  %35 = fmul double %34, %34
  %36 = fmul double %35, 1.010000e+00
  %37 = fmul double %36, %36
  %38 = fmul double %37, 1.010000e+00
  %39 = fmul double %38, %38
  %40 = fmul double %39, 1.010000e+00
  %41 = fmul double %40, %40
  %42 = fmul double %41, 1.010000e+00
  %43 = fmul double %42, %42
  %44 = fmul double %43, 1.010000e+00
  %45 = fmul double %44, %44
  %46 = fmul double %45, 1.010000e+00
  %47 = fmul double %46, %46
  %48 = fmul double %47, 1.010000e+00
  %49 = fmul double %48, %48
  %50 = fmul double %49, 1.010000e+00
  %51 = fmul double %50, %50
  %52 = fmul double %51, 1.010000e+00
  %53 = fmul double %52, %52
  %54 = fmul double %53, 1.010000e+00
  %55 = fmul double %54, %54
  %56 = fmul double %55, 1.010000e+00
  %57 = fmul double %56, %56
  %58 = fmul double %57, 1.010000e+00
  %59 = fmul double %58, %58
  %60 = fmul double %59, 1.010000e+00
  %61 = fmul double %60, %60
  %62 = fmul double %61, 1.010000e+00
  %63 = fmul double %62, %62
  %64 = fmul double %63, 1.010000e+00
  %65 = fmul double %64, %64
  %66 = fmul double %65, 1.010000e+00
  %67 = fmul double %66, %66
  %68 = fmul double %67, 1.010000e+00
  %69 = fmul double %68, %68
  %70 = fmul double %69, 1.010000e+00
  %71 = fmul double %70, %70
  %72 = fmul double %71, 1.010000e+00
  %73 = fmul double %72, %72
  %74 = fmul double %73, 1.010000e+00
  %75 = fmul double %74, %74
  %76 = fmul double %75, 1.010000e+00
  %77 = fmul double %76, %76
  %78 = fmul double %77, 1.010000e+00
  %79 = fmul double %78, %78
  %80 = fmul double %79, 1.010000e+00
  %81 = fmul double %80, %80
  %82 = fmul double %81, 1.010000e+00
  %83 = fmul double %82, %82
  %84 = fmul double %83, 1.010000e+00
  %85 = fmul double %84, %84
  %86 = fmul double %85, 1.010000e+00
  %87 = fmul double %86, %86
  %88 = fmul double %87, 1.010000e+00
  %89 = fmul double %88, %88
  %90 = fmul double %89, 1.010000e+00
  %91 = fmul double %90, %90
  %92 = fmul double %91, 1.010000e+00
  %93 = fmul double %92, %92
  %94 = fmul double %93, 1.010000e+00
  %95 = fmul double %94, %94
  %96 = fmul double %95, 1.010000e+00
  %97 = fmul double %96, %96
  %98 = fmul double %97, 1.010000e+00
  %99 = fmul double %98, %98
  %100 = fmul double %99, 1.010000e+00
  %101 = fmul double %100, %100
  %102 = fmul double %101, 1.010000e+00
  %103 = fmul double %102, %102
  %104 = fmul double %103, 1.010000e+00
  %105 = fmul double %104, %104
  %106 = fmul double %105, 1.010000e+00
  %107 = fmul double %106, %106
  %108 = fmul double %107, 1.010000e+00
  %109 = fmul double %108, %108
  %110 = fmul double %109, 1.010000e+00
  %111 = fmul double %110, %110
  %112 = fmul double %111, 1.010000e+00
  %113 = fmul double %112, %112
  %114 = fmul double %113, 1.010000e+00
  %115 = fmul double %114, %114
  %116 = fmul double %115, 1.010000e+00
  %117 = fmul double %116, %116
  %118 = fmul double %117, 1.010000e+00
  %119 = fmul double %118, %118
  %120 = fmul double %119, 1.010000e+00
  %121 = fmul double %120, %120
  %122 = fmul double %121, 1.010000e+00
  %123 = fmul double %122, %122
  %124 = fmul double %123, 1.010000e+00
  %125 = fmul double %124, %124
  %126 = fmul double %125, 1.010000e+00
  %127 = fmul double %126, %126
  %128 = fmul double %127, 1.010000e+00
  %129 = fmul double %128, %128
  %130 = fmul double %129, 1.010000e+00
  %131 = fmul double %130, %130
  %132 = fmul double %131, 1.010000e+00
  %133 = fmul double %132, %132
  %134 = fmul double %133, 1.010000e+00
  %135 = fmul double %134, %134
  %136 = fmul double %135, 1.010000e+00
  %137 = fmul double %136, %136
  %138 = fmul double %137, 1.010000e+00
  %139 = fmul double %138, %138
  %140 = fmul double %139, 1.010000e+00
  %141 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %141, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB3.i
  %s.0.lcssa.i = phi double [ %24, %SyncBB3.i ], [ %140, %bb.nph.i ]
  store double %s.0.lcssa.i, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Mul1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__Mul1_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Mul1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_Mul1_locals_anchor", void (i8*)* @Mul1}
!1 = metadata !{i32 0, i32 0, i32 0}
