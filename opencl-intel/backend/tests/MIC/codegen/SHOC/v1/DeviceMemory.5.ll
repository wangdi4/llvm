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

declare void @__writeGlobalMemoryCoalesced_original(float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__writeGlobalMemoryCoalesced_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %7 = sitofp i32 %6 to float
  br label %8

; <label>:8                                       ; preds = %8, %SyncBB3
  %s.02 = phi i32 [ %6, %SyncBB3 ], [ %74, %8 ]
  %j.01 = phi i32 [ 0, %SyncBB3 ], [ %72, %8 ]
  %9 = and i32 %s.02, %0
  %10 = sext i32 %9 to i64
  %11 = getelementptr inbounds float addrspace(1)* %output, i64 %10
  store float %7, float addrspace(1)* %11, align 4
  %12 = add nsw i32 %s.02, 32
  %13 = and i32 %12, %0
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds float addrspace(1)* %output, i64 %14
  store float %7, float addrspace(1)* %15, align 4
  %16 = add nsw i32 %s.02, 64
  %17 = and i32 %16, %0
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds float addrspace(1)* %output, i64 %18
  store float %7, float addrspace(1)* %19, align 4
  %20 = add nsw i32 %s.02, 96
  %21 = and i32 %20, %0
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds float addrspace(1)* %output, i64 %22
  store float %7, float addrspace(1)* %23, align 4
  %24 = add nsw i32 %s.02, 128
  %25 = and i32 %24, %0
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds float addrspace(1)* %output, i64 %26
  store float %7, float addrspace(1)* %27, align 4
  %28 = add nsw i32 %s.02, 160
  %29 = and i32 %28, %0
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds float addrspace(1)* %output, i64 %30
  store float %7, float addrspace(1)* %31, align 4
  %32 = add nsw i32 %s.02, 192
  %33 = and i32 %32, %0
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds float addrspace(1)* %output, i64 %34
  store float %7, float addrspace(1)* %35, align 4
  %36 = add nsw i32 %s.02, 224
  %37 = and i32 %36, %0
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds float addrspace(1)* %output, i64 %38
  store float %7, float addrspace(1)* %39, align 4
  %40 = add nsw i32 %s.02, 256
  %41 = and i32 %40, %0
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds float addrspace(1)* %output, i64 %42
  store float %7, float addrspace(1)* %43, align 4
  %44 = add nsw i32 %s.02, 288
  %45 = and i32 %44, %0
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds float addrspace(1)* %output, i64 %46
  store float %7, float addrspace(1)* %47, align 4
  %48 = add nsw i32 %s.02, 320
  %49 = and i32 %48, %0
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds float addrspace(1)* %output, i64 %50
  store float %7, float addrspace(1)* %51, align 4
  %52 = add nsw i32 %s.02, 352
  %53 = and i32 %52, %0
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds float addrspace(1)* %output, i64 %54
  store float %7, float addrspace(1)* %55, align 4
  %56 = add nsw i32 %s.02, 384
  %57 = and i32 %56, %0
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds float addrspace(1)* %output, i64 %58
  store float %7, float addrspace(1)* %59, align 4
  %60 = add nsw i32 %s.02, 416
  %61 = and i32 %60, %0
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %output, i64 %62
  store float %7, float addrspace(1)* %63, align 4
  %64 = add nsw i32 %s.02, 448
  %65 = and i32 %64, %0
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %output, i64 %66
  store float %7, float addrspace(1)* %67, align 4
  %68 = add nsw i32 %s.02, 480
  %69 = and i32 %68, %0
  %70 = sext i32 %69 to i64
  %71 = getelementptr inbounds float addrspace(1)* %output, i64 %70
  store float %7, float addrspace(1)* %71, align 4
  %72 = add nsw i32 %j.01, 1
  %73 = add nsw i32 %s.02, 512
  %74 = and i32 %73, %0
  %exitcond = icmp eq i32 %72, 1024
  br i1 %exitcond, label %._crit_edge, label %8

._crit_edge:                                      ; preds = %8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB3

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

define void @writeGlobalMemoryCoalesced(i8* %pBuffer) {
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
  %21 = sitofp i32 %20 to float
  br label %22

; <label>:22                                      ; preds = %22, %SyncBB3.i
  %s.02.i = phi i32 [ %20, %SyncBB3.i ], [ %88, %22 ]
  %j.01.i = phi i32 [ 0, %SyncBB3.i ], [ %86, %22 ]
  %23 = and i32 %s.02.i, %14
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds float addrspace(1)* %1, i64 %24
  store float %21, float addrspace(1)* %25, align 4
  %26 = add nsw i32 %s.02.i, 32
  %27 = and i32 %26, %14
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds float addrspace(1)* %1, i64 %28
  store float %21, float addrspace(1)* %29, align 4
  %30 = add nsw i32 %s.02.i, 64
  %31 = and i32 %30, %14
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds float addrspace(1)* %1, i64 %32
  store float %21, float addrspace(1)* %33, align 4
  %34 = add nsw i32 %s.02.i, 96
  %35 = and i32 %34, %14
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %36
  store float %21, float addrspace(1)* %37, align 4
  %38 = add nsw i32 %s.02.i, 128
  %39 = and i32 %38, %14
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds float addrspace(1)* %1, i64 %40
  store float %21, float addrspace(1)* %41, align 4
  %42 = add nsw i32 %s.02.i, 160
  %43 = and i32 %42, %14
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %44
  store float %21, float addrspace(1)* %45, align 4
  %46 = add nsw i32 %s.02.i, 192
  %47 = and i32 %46, %14
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds float addrspace(1)* %1, i64 %48
  store float %21, float addrspace(1)* %49, align 4
  %50 = add nsw i32 %s.02.i, 224
  %51 = and i32 %50, %14
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds float addrspace(1)* %1, i64 %52
  store float %21, float addrspace(1)* %53, align 4
  %54 = add nsw i32 %s.02.i, 256
  %55 = and i32 %54, %14
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %56
  store float %21, float addrspace(1)* %57, align 4
  %58 = add nsw i32 %s.02.i, 288
  %59 = and i32 %58, %14
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds float addrspace(1)* %1, i64 %60
  store float %21, float addrspace(1)* %61, align 4
  %62 = add nsw i32 %s.02.i, 320
  %63 = and i32 %62, %14
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds float addrspace(1)* %1, i64 %64
  store float %21, float addrspace(1)* %65, align 4
  %66 = add nsw i32 %s.02.i, 352
  %67 = and i32 %66, %14
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds float addrspace(1)* %1, i64 %68
  store float %21, float addrspace(1)* %69, align 4
  %70 = add nsw i32 %s.02.i, 384
  %71 = and i32 %70, %14
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds float addrspace(1)* %1, i64 %72
  store float %21, float addrspace(1)* %73, align 4
  %74 = add nsw i32 %s.02.i, 416
  %75 = and i32 %74, %14
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %76
  store float %21, float addrspace(1)* %77, align 4
  %78 = add nsw i32 %s.02.i, 448
  %79 = and i32 %78, %14
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %1, i64 %80
  store float %21, float addrspace(1)* %81, align 4
  %82 = add nsw i32 %s.02.i, 480
  %83 = and i32 %82, %14
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds float addrspace(1)* %1, i64 %84
  store float %21, float addrspace(1)* %85, align 4
  %86 = add nsw i32 %j.01.i, 1
  %87 = add nsw i32 %s.02.i, 512
  %88 = and i32 %87, %14
  %exitcond.i = icmp eq i32 %86, 1024
  br i1 %exitcond.i, label %._crit_edge.i, label %22

._crit_edge.i:                                    ; preds = %22
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__writeGlobalMemoryCoalesced_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB3.i

__writeGlobalMemoryCoalesced_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeGlobalMemoryCoalesced_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeGlobalMemoryCoalesced_locals_anchor", void (i8*)* @writeGlobalMemoryCoalesced}
!1 = metadata !{i32 0, i32 0, i32 0}
