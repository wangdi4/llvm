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

declare void @__readGlobalMemoryUnit_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__readGlobalMemoryUnit_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %7 = shl i32 %6, 10
  br label %8

; <label>:8                                       ; preds = %8, %SyncBB
  %s.03 = phi i32 [ %7, %SyncBB ], [ %106, %8 ]
  %sum.02 = phi float [ 0.000000e+00, %SyncBB ], [ %103, %8 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %104, %8 ]
  %9 = and i32 %s.03, %0
  %10 = sext i32 %9 to i64
  %11 = getelementptr inbounds float addrspace(1)* %data, i64 %10
  %12 = load float addrspace(1)* %11, align 4
  %13 = add nsw i32 %s.03, 1
  %14 = and i32 %13, %0
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds float addrspace(1)* %data, i64 %15
  %17 = load float addrspace(1)* %16, align 4
  %18 = add nsw i32 %s.03, 2
  %19 = and i32 %18, %0
  %20 = sext i32 %19 to i64
  %21 = getelementptr inbounds float addrspace(1)* %data, i64 %20
  %22 = load float addrspace(1)* %21, align 4
  %23 = add nsw i32 %s.03, 3
  %24 = and i32 %23, %0
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds float addrspace(1)* %data, i64 %25
  %27 = load float addrspace(1)* %26, align 4
  %28 = add nsw i32 %s.03, 4
  %29 = and i32 %28, %0
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds float addrspace(1)* %data, i64 %30
  %32 = load float addrspace(1)* %31, align 4
  %33 = add nsw i32 %s.03, 5
  %34 = and i32 %33, %0
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds float addrspace(1)* %data, i64 %35
  %37 = load float addrspace(1)* %36, align 4
  %38 = add nsw i32 %s.03, 6
  %39 = and i32 %38, %0
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds float addrspace(1)* %data, i64 %40
  %42 = load float addrspace(1)* %41, align 4
  %43 = add nsw i32 %s.03, 7
  %44 = and i32 %43, %0
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds float addrspace(1)* %data, i64 %45
  %47 = load float addrspace(1)* %46, align 4
  %48 = add nsw i32 %s.03, 8
  %49 = and i32 %48, %0
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds float addrspace(1)* %data, i64 %50
  %52 = load float addrspace(1)* %51, align 4
  %53 = add nsw i32 %s.03, 9
  %54 = and i32 %53, %0
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds float addrspace(1)* %data, i64 %55
  %57 = load float addrspace(1)* %56, align 4
  %58 = add nsw i32 %s.03, 10
  %59 = and i32 %58, %0
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds float addrspace(1)* %data, i64 %60
  %62 = load float addrspace(1)* %61, align 4
  %63 = add nsw i32 %s.03, 11
  %64 = and i32 %63, %0
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %data, i64 %65
  %67 = load float addrspace(1)* %66, align 4
  %68 = add nsw i32 %s.03, 12
  %69 = and i32 %68, %0
  %70 = sext i32 %69 to i64
  %71 = getelementptr inbounds float addrspace(1)* %data, i64 %70
  %72 = load float addrspace(1)* %71, align 4
  %73 = add nsw i32 %s.03, 13
  %74 = and i32 %73, %0
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds float addrspace(1)* %data, i64 %75
  %77 = load float addrspace(1)* %76, align 4
  %78 = add nsw i32 %s.03, 14
  %79 = and i32 %78, %0
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %data, i64 %80
  %82 = load float addrspace(1)* %81, align 4
  %83 = add nsw i32 %s.03, 15
  %84 = and i32 %83, %0
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds float addrspace(1)* %data, i64 %85
  %87 = load float addrspace(1)* %86, align 4
  %88 = fadd float %12, %17
  %89 = fadd float %88, %22
  %90 = fadd float %89, %27
  %91 = fadd float %90, %32
  %92 = fadd float %91, %37
  %93 = fadd float %92, %42
  %94 = fadd float %93, %47
  %95 = fadd float %94, %52
  %96 = fadd float %95, %57
  %97 = fadd float %96, %62
  %98 = fadd float %97, %67
  %99 = fadd float %98, %72
  %100 = fadd float %99, %77
  %101 = fadd float %100, %82
  %102 = fadd float %101, %87
  %103 = fadd float %sum.02, %102
  %104 = add nsw i32 %j.01, 1
  %105 = add nsw i32 %s.03, 16
  %106 = and i32 %105, %0
  %exitcond = icmp eq i32 %104, 512
  br i1 %exitcond, label %._crit_edge, label %8

._crit_edge:                                      ; preds = %8
  %107 = sext i32 %6 to i64
  %108 = getelementptr inbounds float addrspace(1)* %output, i64 %107
  store float %103, float addrspace(1)* %108, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB4

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB4:                                          ; preds = %._crit_edge
  ret void
}

define void @readGlobalMemoryUnit(i8* %pBuffer) {
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
  %24 = shl i32 %23, 10
  br label %25

; <label>:25                                      ; preds = %25, %SyncBB.i
  %s.03.i = phi i32 [ %24, %SyncBB.i ], [ %123, %25 ]
  %sum.02.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %120, %25 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %121, %25 ]
  %26 = and i32 %s.03.i, %17
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds float addrspace(1)* %1, i64 %27
  %29 = load float addrspace(1)* %28, align 4
  %30 = add nsw i32 %s.03.i, 1
  %31 = and i32 %30, %17
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds float addrspace(1)* %1, i64 %32
  %34 = load float addrspace(1)* %33, align 4
  %35 = add nsw i32 %s.03.i, 2
  %36 = and i32 %35, %17
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds float addrspace(1)* %1, i64 %37
  %39 = load float addrspace(1)* %38, align 4
  %40 = add nsw i32 %s.03.i, 3
  %41 = and i32 %40, %17
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %42
  %44 = load float addrspace(1)* %43, align 4
  %45 = add nsw i32 %s.03.i, 4
  %46 = and i32 %45, %17
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds float addrspace(1)* %1, i64 %47
  %49 = load float addrspace(1)* %48, align 4
  %50 = add nsw i32 %s.03.i, 5
  %51 = and i32 %50, %17
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds float addrspace(1)* %1, i64 %52
  %54 = load float addrspace(1)* %53, align 4
  %55 = add nsw i32 %s.03.i, 6
  %56 = and i32 %55, %17
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %57
  %59 = load float addrspace(1)* %58, align 4
  %60 = add nsw i32 %s.03.i, 7
  %61 = and i32 %60, %17
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %62
  %64 = load float addrspace(1)* %63, align 4
  %65 = add nsw i32 %s.03.i, 8
  %66 = and i32 %65, %17
  %67 = sext i32 %66 to i64
  %68 = getelementptr inbounds float addrspace(1)* %1, i64 %67
  %69 = load float addrspace(1)* %68, align 4
  %70 = add nsw i32 %s.03.i, 9
  %71 = and i32 %70, %17
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds float addrspace(1)* %1, i64 %72
  %74 = load float addrspace(1)* %73, align 4
  %75 = add nsw i32 %s.03.i, 10
  %76 = and i32 %75, %17
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %77
  %79 = load float addrspace(1)* %78, align 4
  %80 = add nsw i32 %s.03.i, 11
  %81 = and i32 %80, %17
  %82 = sext i32 %81 to i64
  %83 = getelementptr inbounds float addrspace(1)* %1, i64 %82
  %84 = load float addrspace(1)* %83, align 4
  %85 = add nsw i32 %s.03.i, 12
  %86 = and i32 %85, %17
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds float addrspace(1)* %1, i64 %87
  %89 = load float addrspace(1)* %88, align 4
  %90 = add nsw i32 %s.03.i, 13
  %91 = and i32 %90, %17
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds float addrspace(1)* %1, i64 %92
  %94 = load float addrspace(1)* %93, align 4
  %95 = add nsw i32 %s.03.i, 14
  %96 = and i32 %95, %17
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds float addrspace(1)* %1, i64 %97
  %99 = load float addrspace(1)* %98, align 4
  %100 = add nsw i32 %s.03.i, 15
  %101 = and i32 %100, %17
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds float addrspace(1)* %1, i64 %102
  %104 = load float addrspace(1)* %103, align 4
  %105 = fadd float %29, %34
  %106 = fadd float %105, %39
  %107 = fadd float %106, %44
  %108 = fadd float %107, %49
  %109 = fadd float %108, %54
  %110 = fadd float %109, %59
  %111 = fadd float %110, %64
  %112 = fadd float %111, %69
  %113 = fadd float %112, %74
  %114 = fadd float %113, %79
  %115 = fadd float %114, %84
  %116 = fadd float %115, %89
  %117 = fadd float %116, %94
  %118 = fadd float %117, %99
  %119 = fadd float %118, %104
  %120 = fadd float %sum.02.i, %119
  %121 = add nsw i32 %j.01.i, 1
  %122 = add nsw i32 %s.03.i, 16
  %123 = and i32 %122, %17
  %exitcond.i = icmp eq i32 %121, 512
  br i1 %exitcond.i, label %._crit_edge.i, label %25

._crit_edge.i:                                    ; preds = %25
  %124 = sext i32 %23 to i64
  %125 = getelementptr inbounds float addrspace(1)* %4, i64 %124
  store float %120, float addrspace(1)* %125, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readGlobalMemoryUnit_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__readGlobalMemoryUnit_separated_args.exit:       ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readGlobalMemoryUnit_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int", metadata !"opencl_readGlobalMemoryUnit_locals_anchor", void (i8*)* @readGlobalMemoryUnit}
!1 = metadata !{i32 0, i32 0, i32 0}
