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

declare void @__readGlobalMemoryCoalesced_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__readGlobalMemoryCoalesced_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %0 = add nsw i32 %size, -1
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %6 = trunc i64 %5 to i32
  br label %7

; <label>:7                                       ; preds = %7, %SyncBB
  %s.03 = phi i32 [ %6, %SyncBB ], [ %105, %7 ]
  %sum.02 = phi float [ 0.000000e+00, %SyncBB ], [ %102, %7 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %103, %7 ]
  %8 = and i32 %s.03, %0
  %9 = sext i32 %8 to i64
  %10 = getelementptr inbounds float addrspace(1)* %data, i64 %9
  %11 = load float addrspace(1)* %10, align 4
  %12 = add nsw i32 %s.03, 32
  %13 = and i32 %12, %0
  %14 = sext i32 %13 to i64
  %15 = getelementptr inbounds float addrspace(1)* %data, i64 %14
  %16 = load float addrspace(1)* %15, align 4
  %17 = add nsw i32 %s.03, 64
  %18 = and i32 %17, %0
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds float addrspace(1)* %data, i64 %19
  %21 = load float addrspace(1)* %20, align 4
  %22 = add nsw i32 %s.03, 96
  %23 = and i32 %22, %0
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds float addrspace(1)* %data, i64 %24
  %26 = load float addrspace(1)* %25, align 4
  %27 = add nsw i32 %s.03, 128
  %28 = and i32 %27, %0
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds float addrspace(1)* %data, i64 %29
  %31 = load float addrspace(1)* %30, align 4
  %32 = add nsw i32 %s.03, 160
  %33 = and i32 %32, %0
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds float addrspace(1)* %data, i64 %34
  %36 = load float addrspace(1)* %35, align 4
  %37 = add nsw i32 %s.03, 192
  %38 = and i32 %37, %0
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds float addrspace(1)* %data, i64 %39
  %41 = load float addrspace(1)* %40, align 4
  %42 = add nsw i32 %s.03, 224
  %43 = and i32 %42, %0
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds float addrspace(1)* %data, i64 %44
  %46 = load float addrspace(1)* %45, align 4
  %47 = add nsw i32 %s.03, 256
  %48 = and i32 %47, %0
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds float addrspace(1)* %data, i64 %49
  %51 = load float addrspace(1)* %50, align 4
  %52 = add nsw i32 %s.03, 288
  %53 = and i32 %52, %0
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds float addrspace(1)* %data, i64 %54
  %56 = load float addrspace(1)* %55, align 4
  %57 = add nsw i32 %s.03, 320
  %58 = and i32 %57, %0
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds float addrspace(1)* %data, i64 %59
  %61 = load float addrspace(1)* %60, align 4
  %62 = add nsw i32 %s.03, 352
  %63 = and i32 %62, %0
  %64 = sext i32 %63 to i64
  %65 = getelementptr inbounds float addrspace(1)* %data, i64 %64
  %66 = load float addrspace(1)* %65, align 4
  %67 = add nsw i32 %s.03, 384
  %68 = and i32 %67, %0
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds float addrspace(1)* %data, i64 %69
  %71 = load float addrspace(1)* %70, align 4
  %72 = add nsw i32 %s.03, 416
  %73 = and i32 %72, %0
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds float addrspace(1)* %data, i64 %74
  %76 = load float addrspace(1)* %75, align 4
  %77 = add nsw i32 %s.03, 448
  %78 = and i32 %77, %0
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds float addrspace(1)* %data, i64 %79
  %81 = load float addrspace(1)* %80, align 4
  %82 = add nsw i32 %s.03, 480
  %83 = and i32 %82, %0
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds float addrspace(1)* %data, i64 %84
  %86 = load float addrspace(1)* %85, align 4
  %87 = fadd float %11, %16
  %88 = fadd float %87, %21
  %89 = fadd float %88, %26
  %90 = fadd float %89, %31
  %91 = fadd float %90, %36
  %92 = fadd float %91, %41
  %93 = fadd float %92, %46
  %94 = fadd float %93, %51
  %95 = fadd float %94, %56
  %96 = fadd float %95, %61
  %97 = fadd float %96, %66
  %98 = fadd float %97, %71
  %99 = fadd float %98, %76
  %100 = fadd float %99, %81
  %101 = fadd float %100, %86
  %102 = fadd float %sum.02, %101
  %103 = add nsw i32 %j.01, 1
  %104 = add nsw i32 %s.03, 512
  %105 = and i32 %104, %0
  %exitcond = icmp eq i32 %103, 1024
  br i1 %exitcond, label %._crit_edge, label %7

._crit_edge:                                      ; preds = %7
  %106 = sext i32 %6 to i64
  %107 = getelementptr inbounds float addrspace(1)* %output, i64 %106
  store float %102, float addrspace(1)* %107, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB4

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB4:                                          ; preds = %._crit_edge
  ret void
}

define void @readGlobalMemoryCoalesced(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 48
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = add nsw i32 %7, -1
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %18 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = add i64 %19, %21
  %23 = trunc i64 %22 to i32
  br label %24

; <label>:24                                      ; preds = %24, %SyncBB.i
  %s.03.i = phi i32 [ %23, %SyncBB.i ], [ %122, %24 ]
  %sum.02.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %119, %24 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %120, %24 ]
  %25 = and i32 %s.03.i, %17
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds float addrspace(1)* %1, i64 %26
  %28 = load float addrspace(1)* %27, align 4
  %29 = add nsw i32 %s.03.i, 32
  %30 = and i32 %29, %17
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds float addrspace(1)* %1, i64 %31
  %33 = load float addrspace(1)* %32, align 4
  %34 = add nsw i32 %s.03.i, 64
  %35 = and i32 %34, %17
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %36
  %38 = load float addrspace(1)* %37, align 4
  %39 = add nsw i32 %s.03.i, 96
  %40 = and i32 %39, %17
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %41
  %43 = load float addrspace(1)* %42, align 4
  %44 = add nsw i32 %s.03.i, 128
  %45 = and i32 %44, %17
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds float addrspace(1)* %1, i64 %46
  %48 = load float addrspace(1)* %47, align 4
  %49 = add nsw i32 %s.03.i, 160
  %50 = and i32 %49, %17
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds float addrspace(1)* %1, i64 %51
  %53 = load float addrspace(1)* %52, align 4
  %54 = add nsw i32 %s.03.i, 192
  %55 = and i32 %54, %17
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %56
  %58 = load float addrspace(1)* %57, align 4
  %59 = add nsw i32 %s.03.i, 224
  %60 = and i32 %59, %17
  %61 = sext i32 %60 to i64
  %62 = getelementptr inbounds float addrspace(1)* %1, i64 %61
  %63 = load float addrspace(1)* %62, align 4
  %64 = add nsw i32 %s.03.i, 256
  %65 = and i32 %64, %17
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %1, i64 %66
  %68 = load float addrspace(1)* %67, align 4
  %69 = add nsw i32 %s.03.i, 288
  %70 = and i32 %69, %17
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %1, i64 %71
  %73 = load float addrspace(1)* %72, align 4
  %74 = add nsw i32 %s.03.i, 320
  %75 = and i32 %74, %17
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %76
  %78 = load float addrspace(1)* %77, align 4
  %79 = add nsw i32 %s.03.i, 352
  %80 = and i32 %79, %17
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds float addrspace(1)* %1, i64 %81
  %83 = load float addrspace(1)* %82, align 4
  %84 = add nsw i32 %s.03.i, 384
  %85 = and i32 %84, %17
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds float addrspace(1)* %1, i64 %86
  %88 = load float addrspace(1)* %87, align 4
  %89 = add nsw i32 %s.03.i, 416
  %90 = and i32 %89, %17
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds float addrspace(1)* %1, i64 %91
  %93 = load float addrspace(1)* %92, align 4
  %94 = add nsw i32 %s.03.i, 448
  %95 = and i32 %94, %17
  %96 = sext i32 %95 to i64
  %97 = getelementptr inbounds float addrspace(1)* %1, i64 %96
  %98 = load float addrspace(1)* %97, align 4
  %99 = add nsw i32 %s.03.i, 480
  %100 = and i32 %99, %17
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %1, i64 %101
  %103 = load float addrspace(1)* %102, align 4
  %104 = fadd float %28, %33
  %105 = fadd float %104, %38
  %106 = fadd float %105, %43
  %107 = fadd float %106, %48
  %108 = fadd float %107, %53
  %109 = fadd float %108, %58
  %110 = fadd float %109, %63
  %111 = fadd float %110, %68
  %112 = fadd float %111, %73
  %113 = fadd float %112, %78
  %114 = fadd float %113, %83
  %115 = fadd float %114, %88
  %116 = fadd float %115, %93
  %117 = fadd float %116, %98
  %118 = fadd float %117, %103
  %119 = fadd float %sum.02.i, %118
  %120 = add nsw i32 %j.01.i, 1
  %121 = add nsw i32 %s.03.i, 512
  %122 = and i32 %121, %17
  %exitcond.i = icmp eq i32 %120, 1024
  br i1 %exitcond.i, label %._crit_edge.i, label %24

._crit_edge.i:                                    ; preds = %24
  %123 = sext i32 %23 to i64
  %124 = getelementptr inbounds float addrspace(1)* %4, i64 %123
  store float %119, float addrspace(1)* %124, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readGlobalMemoryCoalesced_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__readGlobalMemoryCoalesced_separated_args.exit:  ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readGlobalMemoryCoalesced_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int", metadata !"opencl_readGlobalMemoryCoalesced_locals_anchor", void (i8*)* @readGlobalMemoryCoalesced}
!1 = metadata !{i32 0, i32 0, i32 0}
