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

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__Mul2_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %9 = fsub double %8, %8
  %10 = fadd double %9, 9.990000e-01
  %11 = fsub double 1.000000e+01, %10
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB6, %bb.nph
  %j.03 = phi i32 [ %128, %bb.nph ], [ 0, %SyncBB6 ]
  %s2.02 = phi double [ %127, %bb.nph ], [ %11, %SyncBB6 ]
  %s.01 = phi double [ %125, %bb.nph ], [ %10, %SyncBB6 ]
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

._crit_edge:                                      ; preds = %bb.nph, %SyncBB6
  %s2.0.lcssa = phi double [ %11, %SyncBB6 ], [ %127, %bb.nph ]
  %s.0.lcssa = phi double [ %10, %SyncBB6 ], [ %125, %bb.nph ]
  %129 = fadd double %s.0.lcssa, %s2.0.lcssa
  store double %129, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB6

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
  %23 = fsub double %22, %22
  %24 = fadd double %23, 9.990000e-01
  %25 = fsub double 1.000000e+01, %24
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB6.i
  %j.03.i = phi i32 [ %142, %bb.nph.i ], [ 0, %SyncBB6.i ]
  %s2.02.i = phi double [ %141, %bb.nph.i ], [ %25, %SyncBB6.i ]
  %s.01.i = phi double [ %139, %bb.nph.i ], [ %24, %SyncBB6.i ]
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

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB6.i
  %s2.0.lcssa.i = phi double [ %25, %SyncBB6.i ], [ %141, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %24, %SyncBB6.i ], [ %139, %bb.nph.i ]
  %143 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  store double %143, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Mul2_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB6.i

__Mul2_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Mul2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_Mul2_locals_anchor", void (i8*)* @Mul2}
!1 = metadata !{i32 0, i32 0, i32 0}
