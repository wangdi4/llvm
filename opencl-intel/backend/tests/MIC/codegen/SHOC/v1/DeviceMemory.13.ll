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

declare void @__writeGlobalMemoryUnit_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__writeGlobalMemoryUnit_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %0 = add nsw i32 %size, -1
  br label %SyncBB3

SyncBB3:                                          ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  %7 = shl i32 %6, 10
  %8 = sitofp i32 %6 to float
  br label %9

; <label>:9                                       ; preds = %9, %SyncBB3
  %s.02 = phi i32 [ %7, %SyncBB3 ], [ %75, %9 ]
  %j.01 = phi i32 [ 0, %SyncBB3 ], [ %73, %9 ]
  %10 = and i32 %s.02, %0
  %11 = sext i32 %10 to i64
  %12 = getelementptr inbounds float addrspace(1)* %output, i64 %11
  store float %8, float addrspace(1)* %12, align 4
  %13 = add nsw i32 %s.02, 1
  %14 = and i32 %13, %0
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds float addrspace(1)* %output, i64 %15
  store float %8, float addrspace(1)* %16, align 4
  %17 = add nsw i32 %s.02, 2
  %18 = and i32 %17, %0
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds float addrspace(1)* %output, i64 %19
  store float %8, float addrspace(1)* %20, align 4
  %21 = add nsw i32 %s.02, 3
  %22 = and i32 %21, %0
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds float addrspace(1)* %output, i64 %23
  store float %8, float addrspace(1)* %24, align 4
  %25 = add nsw i32 %s.02, 4
  %26 = and i32 %25, %0
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds float addrspace(1)* %output, i64 %27
  store float %8, float addrspace(1)* %28, align 4
  %29 = add nsw i32 %s.02, 5
  %30 = and i32 %29, %0
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds float addrspace(1)* %output, i64 %31
  store float %8, float addrspace(1)* %32, align 4
  %33 = add nsw i32 %s.02, 6
  %34 = and i32 %33, %0
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds float addrspace(1)* %output, i64 %35
  store float %8, float addrspace(1)* %36, align 4
  %37 = add nsw i32 %s.02, 7
  %38 = and i32 %37, %0
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds float addrspace(1)* %output, i64 %39
  store float %8, float addrspace(1)* %40, align 4
  %41 = add nsw i32 %s.02, 8
  %42 = and i32 %41, %0
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds float addrspace(1)* %output, i64 %43
  store float %8, float addrspace(1)* %44, align 4
  %45 = add nsw i32 %s.02, 9
  %46 = and i32 %45, %0
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds float addrspace(1)* %output, i64 %47
  store float %8, float addrspace(1)* %48, align 4
  %49 = add nsw i32 %s.02, 10
  %50 = and i32 %49, %0
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds float addrspace(1)* %output, i64 %51
  store float %8, float addrspace(1)* %52, align 4
  %53 = add nsw i32 %s.02, 11
  %54 = and i32 %53, %0
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds float addrspace(1)* %output, i64 %55
  store float %8, float addrspace(1)* %56, align 4
  %57 = add nsw i32 %s.02, 12
  %58 = and i32 %57, %0
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds float addrspace(1)* %output, i64 %59
  store float %8, float addrspace(1)* %60, align 4
  %61 = add nsw i32 %s.02, 13
  %62 = and i32 %61, %0
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds float addrspace(1)* %output, i64 %63
  store float %8, float addrspace(1)* %64, align 4
  %65 = add nsw i32 %s.02, 14
  %66 = and i32 %65, %0
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds float addrspace(1)* %output, i64 %67
  store float %8, float addrspace(1)* %68, align 4
  %69 = add nsw i32 %s.02, 15
  %70 = and i32 %69, %0
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %output, i64 %71
  store float %8, float addrspace(1)* %72, align 4
  %73 = add nsw i32 %j.01, 1
  %74 = add nsw i32 %s.02, 16
  %75 = and i32 %74, %0
  %exitcond = icmp eq i32 %73, 512
  br i1 %exitcond, label %._crit_edge, label %9

._crit_edge:                                      ; preds = %9
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @writeGlobalMemoryUnit(i8* %pBuffer) {
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
  %14 = add nsw i32 %4, -1
  br label %SyncBB3.i

SyncBB3.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %20 = trunc i64 %19 to i32
  %21 = shl i32 %20, 10
  %22 = sitofp i32 %20 to float
  br label %23

; <label>:23                                      ; preds = %23, %SyncBB3.i
  %s.02.i = phi i32 [ %21, %SyncBB3.i ], [ %89, %23 ]
  %j.01.i = phi i32 [ 0, %SyncBB3.i ], [ %87, %23 ]
  %24 = and i32 %s.02.i, %14
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds float addrspace(1)* %1, i64 %25
  store float %22, float addrspace(1)* %26, align 4
  %27 = add nsw i32 %s.02.i, 1
  %28 = and i32 %27, %14
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds float addrspace(1)* %1, i64 %29
  store float %22, float addrspace(1)* %30, align 4
  %31 = add nsw i32 %s.02.i, 2
  %32 = and i32 %31, %14
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds float addrspace(1)* %1, i64 %33
  store float %22, float addrspace(1)* %34, align 4
  %35 = add nsw i32 %s.02.i, 3
  %36 = and i32 %35, %14
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %37
  store float %22, float addrspace(1)* %38, align 4
  %39 = add nsw i32 %s.02.i, 4
  %40 = and i32 %39, %14
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %41
  store float %22, float addrspace(1)* %42, align 4
  %43 = add nsw i32 %s.02.i, 5
  %44 = and i32 %43, %14
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds float addrspace(1)* %1, i64 %45
  store float %22, float addrspace(1)* %46, align 4
  %47 = add nsw i32 %s.02.i, 6
  %48 = and i32 %47, %14
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds float addrspace(1)* %1, i64 %49
  store float %22, float addrspace(1)* %50, align 4
  %51 = add nsw i32 %s.02.i, 7
  %52 = and i32 %51, %14
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds float addrspace(1)* %1, i64 %53
  store float %22, float addrspace(1)* %54, align 4
  %55 = add nsw i32 %s.02.i, 8
  %56 = and i32 %55, %14
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %57
  store float %22, float addrspace(1)* %58, align 4
  %59 = add nsw i32 %s.02.i, 9
  %60 = and i32 %59, %14
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %61
  store float %22, float addrspace(1)* %62, align 4
  %63 = add nsw i32 %s.02.i, 10
  %64 = and i32 %63, %14
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %65
  store float %22, float addrspace(1)* %66, align 4
  %67 = add nsw i32 %s.02.i, 11
  %68 = and i32 %67, %14
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds float addrspace(1)* %1, i64 %69
  store float %22, float addrspace(1)* %70, align 4
  %71 = add nsw i32 %s.02.i, 12
  %72 = and i32 %71, %14
  %73 = sext i32 %72 to i64
  %74 = getelementptr inbounds float addrspace(1)* %1, i64 %73
  store float %22, float addrspace(1)* %74, align 4
  %75 = add nsw i32 %s.02.i, 13
  %76 = and i32 %75, %14
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %77
  store float %22, float addrspace(1)* %78, align 4
  %79 = add nsw i32 %s.02.i, 14
  %80 = and i32 %79, %14
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds float addrspace(1)* %1, i64 %81
  store float %22, float addrspace(1)* %82, align 4
  %83 = add nsw i32 %s.02.i, 15
  %84 = and i32 %83, %14
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds float addrspace(1)* %1, i64 %85
  store float %22, float addrspace(1)* %86, align 4
  %87 = add nsw i32 %j.01.i, 1
  %88 = add nsw i32 %s.02.i, 16
  %89 = and i32 %88, %14
  %exitcond.i = icmp eq i32 %87, 512
  br i1 %exitcond.i, label %._crit_edge.i, label %23

._crit_edge.i:                                    ; preds = %23
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__writeGlobalMemoryUnit_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__writeGlobalMemoryUnit_separated_args.exit:      ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeGlobalMemoryUnit_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeGlobalMemoryUnit_locals_anchor", void (i8*)* @writeGlobalMemoryUnit}
!1 = metadata !{i32 0, i32 0, i32 0}
