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

declare void @__Add8_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__Add8_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
FirstBB:
  %0 = icmp sgt i32 %nIters, 0
  br label %SyncBB24

SyncBB24:                                         ; preds = %thenBB, %FirstBB
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

bb.nph:                                           ; preds = %SyncBB24, %bb.nph
  %j.09 = phi i32 [ %82, %bb.nph ], [ 0, %SyncBB24 ]
  %s8.08 = phi double [ %81, %bb.nph ], [ %9, %SyncBB24 ]
  %s7.07 = phi double [ %80, %bb.nph ], [ %8, %SyncBB24 ]
  %s6.06 = phi double [ %79, %bb.nph ], [ %9, %SyncBB24 ]
  %s.05 = phi double [ %74, %bb.nph ], [ %8, %SyncBB24 ]
  %s2.04 = phi double [ %75, %bb.nph ], [ %9, %SyncBB24 ]
  %s3.03 = phi double [ %76, %bb.nph ], [ %8, %SyncBB24 ]
  %s4.02 = phi double [ %77, %bb.nph ], [ %9, %SyncBB24 ]
  %s5.01 = phi double [ %78, %bb.nph ], [ %8, %SyncBB24 ]
  %10 = fsub double 1.000000e+01, %s.05
  %11 = fsub double 1.000000e+01, %s2.04
  %12 = fsub double 1.000000e+01, %s3.03
  %13 = fsub double 1.000000e+01, %s4.02
  %14 = fsub double 1.000000e+01, %s5.01
  %15 = fsub double 1.000000e+01, %s6.06
  %16 = fsub double 1.000000e+01, %s7.07
  %17 = fsub double 1.000000e+01, %s8.08
  %18 = fsub double 1.000000e+01, %10
  %19 = fsub double 1.000000e+01, %11
  %20 = fsub double 1.000000e+01, %12
  %21 = fsub double 1.000000e+01, %13
  %22 = fsub double 1.000000e+01, %14
  %23 = fsub double 1.000000e+01, %15
  %24 = fsub double 1.000000e+01, %16
  %25 = fsub double 1.000000e+01, %17
  %26 = fsub double 1.000000e+01, %18
  %27 = fsub double 1.000000e+01, %19
  %28 = fsub double 1.000000e+01, %20
  %29 = fsub double 1.000000e+01, %21
  %30 = fsub double 1.000000e+01, %22
  %31 = fsub double 1.000000e+01, %23
  %32 = fsub double 1.000000e+01, %24
  %33 = fsub double 1.000000e+01, %25
  %34 = fsub double 1.000000e+01, %26
  %35 = fsub double 1.000000e+01, %27
  %36 = fsub double 1.000000e+01, %28
  %37 = fsub double 1.000000e+01, %29
  %38 = fsub double 1.000000e+01, %30
  %39 = fsub double 1.000000e+01, %31
  %40 = fsub double 1.000000e+01, %32
  %41 = fsub double 1.000000e+01, %33
  %42 = fsub double 1.000000e+01, %34
  %43 = fsub double 1.000000e+01, %35
  %44 = fsub double 1.000000e+01, %36
  %45 = fsub double 1.000000e+01, %37
  %46 = fsub double 1.000000e+01, %38
  %47 = fsub double 1.000000e+01, %39
  %48 = fsub double 1.000000e+01, %40
  %49 = fsub double 1.000000e+01, %41
  %50 = fsub double 1.000000e+01, %42
  %51 = fsub double 1.000000e+01, %43
  %52 = fsub double 1.000000e+01, %44
  %53 = fsub double 1.000000e+01, %45
  %54 = fsub double 1.000000e+01, %46
  %55 = fsub double 1.000000e+01, %47
  %56 = fsub double 1.000000e+01, %48
  %57 = fsub double 1.000000e+01, %49
  %58 = fsub double 1.000000e+01, %50
  %59 = fsub double 1.000000e+01, %51
  %60 = fsub double 1.000000e+01, %52
  %61 = fsub double 1.000000e+01, %53
  %62 = fsub double 1.000000e+01, %54
  %63 = fsub double 1.000000e+01, %55
  %64 = fsub double 1.000000e+01, %56
  %65 = fsub double 1.000000e+01, %57
  %66 = fsub double 1.000000e+01, %58
  %67 = fsub double 1.000000e+01, %59
  %68 = fsub double 1.000000e+01, %60
  %69 = fsub double 1.000000e+01, %61
  %70 = fsub double 1.000000e+01, %62
  %71 = fsub double 1.000000e+01, %63
  %72 = fsub double 1.000000e+01, %64
  %73 = fsub double 1.000000e+01, %65
  %74 = fsub double 1.000000e+01, %66
  %75 = fsub double 1.000000e+01, %67
  %76 = fsub double 1.000000e+01, %68
  %77 = fsub double 1.000000e+01, %69
  %78 = fsub double 1.000000e+01, %70
  %79 = fsub double 1.000000e+01, %71
  %80 = fsub double 1.000000e+01, %72
  %81 = fsub double 1.000000e+01, %73
  %82 = add nsw i32 %j.09, 1
  %exitcond = icmp eq i32 %82, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB24
  %s8.0.lcssa = phi double [ %9, %SyncBB24 ], [ %81, %bb.nph ]
  %s7.0.lcssa = phi double [ %8, %SyncBB24 ], [ %80, %bb.nph ]
  %s6.0.lcssa = phi double [ %9, %SyncBB24 ], [ %79, %bb.nph ]
  %s.0.lcssa = phi double [ %8, %SyncBB24 ], [ %74, %bb.nph ]
  %s2.0.lcssa = phi double [ %9, %SyncBB24 ], [ %75, %bb.nph ]
  %s3.0.lcssa = phi double [ %8, %SyncBB24 ], [ %76, %bb.nph ]
  %s4.0.lcssa = phi double [ %9, %SyncBB24 ], [ %77, %bb.nph ]
  %s5.0.lcssa = phi double [ %8, %SyncBB24 ], [ %78, %bb.nph ]
  %83 = fadd double %s.0.lcssa, %s2.0.lcssa
  %84 = fadd double %83, %s3.0.lcssa
  %85 = fadd double %84, %s4.0.lcssa
  %86 = fadd double %85, %s5.0.lcssa
  %87 = fadd double %86, %s6.0.lcssa
  %88 = fadd double %87, %s7.0.lcssa
  %89 = fadd double %88, %s8.0.lcssa
  store double %89, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB24

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @Add8(i8* %pBuffer) {
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
  br label %SyncBB24.i

SyncBB24.i:                                       ; preds = %thenBB.i, %entry
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

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB24.i
  %j.09.i = phi i32 [ %96, %bb.nph.i ], [ 0, %SyncBB24.i ]
  %s8.08.i = phi double [ %95, %bb.nph.i ], [ %23, %SyncBB24.i ]
  %s7.07.i = phi double [ %94, %bb.nph.i ], [ %22, %SyncBB24.i ]
  %s6.06.i = phi double [ %93, %bb.nph.i ], [ %23, %SyncBB24.i ]
  %s.05.i = phi double [ %88, %bb.nph.i ], [ %22, %SyncBB24.i ]
  %s2.04.i = phi double [ %89, %bb.nph.i ], [ %23, %SyncBB24.i ]
  %s3.03.i = phi double [ %90, %bb.nph.i ], [ %22, %SyncBB24.i ]
  %s4.02.i = phi double [ %91, %bb.nph.i ], [ %23, %SyncBB24.i ]
  %s5.01.i = phi double [ %92, %bb.nph.i ], [ %22, %SyncBB24.i ]
  %24 = fsub double 1.000000e+01, %s.05.i
  %25 = fsub double 1.000000e+01, %s2.04.i
  %26 = fsub double 1.000000e+01, %s3.03.i
  %27 = fsub double 1.000000e+01, %s4.02.i
  %28 = fsub double 1.000000e+01, %s5.01.i
  %29 = fsub double 1.000000e+01, %s6.06.i
  %30 = fsub double 1.000000e+01, %s7.07.i
  %31 = fsub double 1.000000e+01, %s8.08.i
  %32 = fsub double 1.000000e+01, %24
  %33 = fsub double 1.000000e+01, %25
  %34 = fsub double 1.000000e+01, %26
  %35 = fsub double 1.000000e+01, %27
  %36 = fsub double 1.000000e+01, %28
  %37 = fsub double 1.000000e+01, %29
  %38 = fsub double 1.000000e+01, %30
  %39 = fsub double 1.000000e+01, %31
  %40 = fsub double 1.000000e+01, %32
  %41 = fsub double 1.000000e+01, %33
  %42 = fsub double 1.000000e+01, %34
  %43 = fsub double 1.000000e+01, %35
  %44 = fsub double 1.000000e+01, %36
  %45 = fsub double 1.000000e+01, %37
  %46 = fsub double 1.000000e+01, %38
  %47 = fsub double 1.000000e+01, %39
  %48 = fsub double 1.000000e+01, %40
  %49 = fsub double 1.000000e+01, %41
  %50 = fsub double 1.000000e+01, %42
  %51 = fsub double 1.000000e+01, %43
  %52 = fsub double 1.000000e+01, %44
  %53 = fsub double 1.000000e+01, %45
  %54 = fsub double 1.000000e+01, %46
  %55 = fsub double 1.000000e+01, %47
  %56 = fsub double 1.000000e+01, %48
  %57 = fsub double 1.000000e+01, %49
  %58 = fsub double 1.000000e+01, %50
  %59 = fsub double 1.000000e+01, %51
  %60 = fsub double 1.000000e+01, %52
  %61 = fsub double 1.000000e+01, %53
  %62 = fsub double 1.000000e+01, %54
  %63 = fsub double 1.000000e+01, %55
  %64 = fsub double 1.000000e+01, %56
  %65 = fsub double 1.000000e+01, %57
  %66 = fsub double 1.000000e+01, %58
  %67 = fsub double 1.000000e+01, %59
  %68 = fsub double 1.000000e+01, %60
  %69 = fsub double 1.000000e+01, %61
  %70 = fsub double 1.000000e+01, %62
  %71 = fsub double 1.000000e+01, %63
  %72 = fsub double 1.000000e+01, %64
  %73 = fsub double 1.000000e+01, %65
  %74 = fsub double 1.000000e+01, %66
  %75 = fsub double 1.000000e+01, %67
  %76 = fsub double 1.000000e+01, %68
  %77 = fsub double 1.000000e+01, %69
  %78 = fsub double 1.000000e+01, %70
  %79 = fsub double 1.000000e+01, %71
  %80 = fsub double 1.000000e+01, %72
  %81 = fsub double 1.000000e+01, %73
  %82 = fsub double 1.000000e+01, %74
  %83 = fsub double 1.000000e+01, %75
  %84 = fsub double 1.000000e+01, %76
  %85 = fsub double 1.000000e+01, %77
  %86 = fsub double 1.000000e+01, %78
  %87 = fsub double 1.000000e+01, %79
  %88 = fsub double 1.000000e+01, %80
  %89 = fsub double 1.000000e+01, %81
  %90 = fsub double 1.000000e+01, %82
  %91 = fsub double 1.000000e+01, %83
  %92 = fsub double 1.000000e+01, %84
  %93 = fsub double 1.000000e+01, %85
  %94 = fsub double 1.000000e+01, %86
  %95 = fsub double 1.000000e+01, %87
  %96 = add nsw i32 %j.09.i, 1
  %exitcond.i = icmp eq i32 %96, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB24.i
  %s8.0.lcssa.i = phi double [ %23, %SyncBB24.i ], [ %95, %bb.nph.i ]
  %s7.0.lcssa.i = phi double [ %22, %SyncBB24.i ], [ %94, %bb.nph.i ]
  %s6.0.lcssa.i = phi double [ %23, %SyncBB24.i ], [ %93, %bb.nph.i ]
  %s.0.lcssa.i = phi double [ %22, %SyncBB24.i ], [ %88, %bb.nph.i ]
  %s2.0.lcssa.i = phi double [ %23, %SyncBB24.i ], [ %89, %bb.nph.i ]
  %s3.0.lcssa.i = phi double [ %22, %SyncBB24.i ], [ %90, %bb.nph.i ]
  %s4.0.lcssa.i = phi double [ %23, %SyncBB24.i ], [ %91, %bb.nph.i ]
  %s5.0.lcssa.i = phi double [ %22, %SyncBB24.i ], [ %92, %bb.nph.i ]
  %97 = fadd double %s.0.lcssa.i, %s2.0.lcssa.i
  %98 = fadd double %97, %s3.0.lcssa.i
  %99 = fadd double %98, %s4.0.lcssa.i
  %100 = fadd double %99, %s5.0.lcssa.i
  %101 = fadd double %100, %s6.0.lcssa.i
  %102 = fadd double %101, %s7.0.lcssa.i
  %103 = fadd double %102, %s8.0.lcssa.i
  store double %103, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Add8_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB24.i

__Add8_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Add8_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_Add8_locals_anchor", void (i8*)* @Add8}
!1 = metadata !{i32 0, i32 0, i32 0}
