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

declare void @__Add1_original(double addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__Add1_separated_args(double addrspace(1)* nocapture %data, i32 %nIters, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  br i1 %0, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %SyncBB, %bb.nph
  %j.02 = phi i32 [ %79, %bb.nph ], [ 0, %SyncBB ]
  %s.01 = phi double [ %78, %bb.nph ], [ %8, %SyncBB ]
  %9 = fsub double 1.000000e+01, %s.01
  %10 = fsub double 1.000000e+01, %9
  %11 = fsub double 1.000000e+01, %10
  %12 = fsub double 1.000000e+01, %11
  %13 = fsub double 1.000000e+01, %12
  %14 = fsub double 1.000000e+01, %13
  %15 = fsub double 1.000000e+01, %14
  %16 = fsub double 1.000000e+01, %15
  %17 = fsub double 1.000000e+01, %16
  %18 = fsub double 1.000000e+01, %17
  %19 = fsub double 1.000000e+01, %18
  %20 = fsub double 1.000000e+01, %19
  %21 = fsub double 1.000000e+01, %20
  %22 = fsub double 1.000000e+01, %21
  %23 = fsub double 1.000000e+01, %22
  %24 = fsub double 1.000000e+01, %23
  %25 = fsub double 1.000000e+01, %24
  %26 = fsub double 1.000000e+01, %25
  %27 = fsub double 1.000000e+01, %26
  %28 = fsub double 1.000000e+01, %27
  %29 = fsub double 1.000000e+01, %28
  %30 = fsub double 1.000000e+01, %29
  %31 = fsub double 1.000000e+01, %30
  %32 = fsub double 1.000000e+01, %31
  %33 = fsub double 1.000000e+01, %32
  %34 = fsub double 1.000000e+01, %33
  %35 = fsub double 1.000000e+01, %34
  %36 = fsub double 1.000000e+01, %35
  %37 = fsub double 1.000000e+01, %36
  %38 = fsub double 1.000000e+01, %37
  %39 = fsub double 1.000000e+01, %38
  %40 = fsub double 1.000000e+01, %39
  %41 = fsub double 1.000000e+01, %40
  %42 = fsub double 1.000000e+01, %41
  %43 = fsub double 1.000000e+01, %42
  %44 = fsub double 1.000000e+01, %43
  %45 = fsub double 1.000000e+01, %44
  %46 = fsub double 1.000000e+01, %45
  %47 = fsub double 1.000000e+01, %46
  %48 = fsub double 1.000000e+01, %47
  %49 = fsub double 1.000000e+01, %48
  %50 = fsub double 1.000000e+01, %49
  %51 = fsub double 1.000000e+01, %50
  %52 = fsub double 1.000000e+01, %51
  %53 = fsub double 1.000000e+01, %52
  %54 = fsub double 1.000000e+01, %53
  %55 = fsub double 1.000000e+01, %54
  %56 = fsub double 1.000000e+01, %55
  %57 = fsub double 1.000000e+01, %56
  %58 = fsub double 1.000000e+01, %57
  %59 = fsub double 1.000000e+01, %58
  %60 = fsub double 1.000000e+01, %59
  %61 = fsub double 1.000000e+01, %60
  %62 = fsub double 1.000000e+01, %61
  %63 = fsub double 1.000000e+01, %62
  %64 = fsub double 1.000000e+01, %63
  %65 = fsub double 1.000000e+01, %64
  %66 = fsub double 1.000000e+01, %65
  %67 = fsub double 1.000000e+01, %66
  %68 = fsub double 1.000000e+01, %67
  %69 = fsub double 1.000000e+01, %68
  %70 = fsub double 1.000000e+01, %69
  %71 = fsub double 1.000000e+01, %70
  %72 = fsub double 1.000000e+01, %71
  %73 = fsub double 1.000000e+01, %72
  %74 = fsub double 1.000000e+01, %73
  %75 = fsub double 1.000000e+01, %74
  %76 = fsub double 1.000000e+01, %75
  %77 = fsub double 1.000000e+01, %76
  %78 = fsub double 1.000000e+01, %77
  %79 = add nsw i32 %j.02, 1
  %exitcond = icmp eq i32 %79, %nIters
  br i1 %exitcond, label %._crit_edge, label %bb.nph

._crit_edge:                                      ; preds = %bb.nph, %SyncBB
  %s.0.lcssa = phi double [ %8, %SyncBB ], [ %78, %bb.nph ]
  store double %s.0.lcssa, double addrspace(1)* %7, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @Add1(i8* %pBuffer) {
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
  br i1 %14, label %bb.nph.i, label %._crit_edge.i

bb.nph.i:                                         ; preds = %bb.nph.i, %SyncBB.i
  %j.02.i = phi i32 [ %93, %bb.nph.i ], [ 0, %SyncBB.i ]
  %s.01.i = phi double [ %92, %bb.nph.i ], [ %22, %SyncBB.i ]
  %23 = fsub double 1.000000e+01, %s.01.i
  %24 = fsub double 1.000000e+01, %23
  %25 = fsub double 1.000000e+01, %24
  %26 = fsub double 1.000000e+01, %25
  %27 = fsub double 1.000000e+01, %26
  %28 = fsub double 1.000000e+01, %27
  %29 = fsub double 1.000000e+01, %28
  %30 = fsub double 1.000000e+01, %29
  %31 = fsub double 1.000000e+01, %30
  %32 = fsub double 1.000000e+01, %31
  %33 = fsub double 1.000000e+01, %32
  %34 = fsub double 1.000000e+01, %33
  %35 = fsub double 1.000000e+01, %34
  %36 = fsub double 1.000000e+01, %35
  %37 = fsub double 1.000000e+01, %36
  %38 = fsub double 1.000000e+01, %37
  %39 = fsub double 1.000000e+01, %38
  %40 = fsub double 1.000000e+01, %39
  %41 = fsub double 1.000000e+01, %40
  %42 = fsub double 1.000000e+01, %41
  %43 = fsub double 1.000000e+01, %42
  %44 = fsub double 1.000000e+01, %43
  %45 = fsub double 1.000000e+01, %44
  %46 = fsub double 1.000000e+01, %45
  %47 = fsub double 1.000000e+01, %46
  %48 = fsub double 1.000000e+01, %47
  %49 = fsub double 1.000000e+01, %48
  %50 = fsub double 1.000000e+01, %49
  %51 = fsub double 1.000000e+01, %50
  %52 = fsub double 1.000000e+01, %51
  %53 = fsub double 1.000000e+01, %52
  %54 = fsub double 1.000000e+01, %53
  %55 = fsub double 1.000000e+01, %54
  %56 = fsub double 1.000000e+01, %55
  %57 = fsub double 1.000000e+01, %56
  %58 = fsub double 1.000000e+01, %57
  %59 = fsub double 1.000000e+01, %58
  %60 = fsub double 1.000000e+01, %59
  %61 = fsub double 1.000000e+01, %60
  %62 = fsub double 1.000000e+01, %61
  %63 = fsub double 1.000000e+01, %62
  %64 = fsub double 1.000000e+01, %63
  %65 = fsub double 1.000000e+01, %64
  %66 = fsub double 1.000000e+01, %65
  %67 = fsub double 1.000000e+01, %66
  %68 = fsub double 1.000000e+01, %67
  %69 = fsub double 1.000000e+01, %68
  %70 = fsub double 1.000000e+01, %69
  %71 = fsub double 1.000000e+01, %70
  %72 = fsub double 1.000000e+01, %71
  %73 = fsub double 1.000000e+01, %72
  %74 = fsub double 1.000000e+01, %73
  %75 = fsub double 1.000000e+01, %74
  %76 = fsub double 1.000000e+01, %75
  %77 = fsub double 1.000000e+01, %76
  %78 = fsub double 1.000000e+01, %77
  %79 = fsub double 1.000000e+01, %78
  %80 = fsub double 1.000000e+01, %79
  %81 = fsub double 1.000000e+01, %80
  %82 = fsub double 1.000000e+01, %81
  %83 = fsub double 1.000000e+01, %82
  %84 = fsub double 1.000000e+01, %83
  %85 = fsub double 1.000000e+01, %84
  %86 = fsub double 1.000000e+01, %85
  %87 = fsub double 1.000000e+01, %86
  %88 = fsub double 1.000000e+01, %87
  %89 = fsub double 1.000000e+01, %88
  %90 = fsub double 1.000000e+01, %89
  %91 = fsub double 1.000000e+01, %90
  %92 = fsub double 1.000000e+01, %91
  %93 = add nsw i32 %j.02.i, 1
  %exitcond.i = icmp eq i32 %93, %4
  br i1 %exitcond.i, label %._crit_edge.i, label %bb.nph.i

._crit_edge.i:                                    ; preds = %bb.nph.i, %SyncBB.i
  %s.0.lcssa.i = phi double [ %22, %SyncBB.i ], [ %92, %bb.nph.i ]
  store double %s.0.lcssa.i, double addrspace(1)* %21, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Add1_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__Add1_separated_args.exit:                       ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (double addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Add1_separated_args, metadata !1, metadata !1, metadata !"", metadata !"double __attribute__((address_space(1))) *, int", metadata !"opencl_Add1_locals_anchor", void (i8*)* @Add1}
!1 = metadata !{i32 0, i32 0, i32 0}
