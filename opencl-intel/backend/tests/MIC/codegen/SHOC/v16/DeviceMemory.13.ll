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

declare void @____Vectorized_.writeGlobalMemoryUnit_original(float addrspace(1)* nocapture, i32) nounwind

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

define void @__writeGlobalMemoryUnit_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %8 = sitofp i32 %6 to float
  br label %9

; <label>:9                                       ; preds = %9, %SyncBB
  %s.02 = phi i32 [ %7, %SyncBB ], [ %75, %9 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %73, %9 ]
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
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.writeGlobalMemoryUnit_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph:
  %0 = add nsw i32 %size, -1
  %temp = insertelement <16 x i32> undef, i32 %0, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %bb.nph
  %CurrWI..0 = phi i64 [ 0, %bb.nph ], [ %"CurrWI++", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %broadcast1 = insertelement <16 x i64> undef, i64 %5, i32 0
  %broadcast2 = shufflevector <16 x i64> %broadcast1, <16 x i64> undef, <16 x i32> zeroinitializer
  %6 = add <16 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %7 = trunc <16 x i64> %6 to <16 x i32>
  %8 = shl <16 x i32> %7, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %9 = sitofp <16 x i32> %7 to <16 x float>
  %extract16 = extractelement <16 x float> %9, i32 0
  %extract17 = extractelement <16 x float> %9, i32 1
  %extract18 = extractelement <16 x float> %9, i32 2
  %extract19 = extractelement <16 x float> %9, i32 3
  %extract20 = extractelement <16 x float> %9, i32 4
  %extract21 = extractelement <16 x float> %9, i32 5
  %extract22 = extractelement <16 x float> %9, i32 6
  %extract23 = extractelement <16 x float> %9, i32 7
  %extract24 = extractelement <16 x float> %9, i32 8
  %extract25 = extractelement <16 x float> %9, i32 9
  %extract26 = extractelement <16 x float> %9, i32 10
  %extract27 = extractelement <16 x float> %9, i32 11
  %extract28 = extractelement <16 x float> %9, i32 12
  %extract29 = extractelement <16 x float> %9, i32 13
  %extract30 = extractelement <16 x float> %9, i32 14
  %extract31 = extractelement <16 x float> %9, i32 15
  br label %10

; <label>:10                                      ; preds = %10, %SyncBB
  %vectorPHI = phi <16 x i32> [ %8, %SyncBB ], [ %812, %10 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %810, %10 ]
  %11 = and <16 x i32> %vectorPHI, %vector
  %12 = extractelement <16 x i32> %11, i32 0
  %13 = sext i32 %12 to i64
  %14 = getelementptr inbounds float addrspace(1)* %output, i64 %13
  %15 = extractelement <16 x i32> %11, i32 1
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds float addrspace(1)* %output, i64 %16
  %18 = extractelement <16 x i32> %11, i32 2
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds float addrspace(1)* %output, i64 %19
  %21 = extractelement <16 x i32> %11, i32 3
  %22 = sext i32 %21 to i64
  %23 = getelementptr inbounds float addrspace(1)* %output, i64 %22
  %24 = extractelement <16 x i32> %11, i32 4
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds float addrspace(1)* %output, i64 %25
  %27 = extractelement <16 x i32> %11, i32 5
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds float addrspace(1)* %output, i64 %28
  %30 = extractelement <16 x i32> %11, i32 6
  %31 = sext i32 %30 to i64
  %32 = getelementptr inbounds float addrspace(1)* %output, i64 %31
  %33 = extractelement <16 x i32> %11, i32 7
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds float addrspace(1)* %output, i64 %34
  %36 = extractelement <16 x i32> %11, i32 8
  %37 = sext i32 %36 to i64
  %38 = getelementptr inbounds float addrspace(1)* %output, i64 %37
  %39 = extractelement <16 x i32> %11, i32 9
  %40 = sext i32 %39 to i64
  %41 = getelementptr inbounds float addrspace(1)* %output, i64 %40
  %42 = extractelement <16 x i32> %11, i32 10
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds float addrspace(1)* %output, i64 %43
  %45 = extractelement <16 x i32> %11, i32 11
  %46 = sext i32 %45 to i64
  %47 = getelementptr inbounds float addrspace(1)* %output, i64 %46
  %48 = extractelement <16 x i32> %11, i32 12
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds float addrspace(1)* %output, i64 %49
  %51 = extractelement <16 x i32> %11, i32 13
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds float addrspace(1)* %output, i64 %52
  %54 = extractelement <16 x i32> %11, i32 14
  %55 = sext i32 %54 to i64
  %56 = getelementptr inbounds float addrspace(1)* %output, i64 %55
  %57 = extractelement <16 x i32> %11, i32 15
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds float addrspace(1)* %output, i64 %58
  store float %extract16, float addrspace(1)* %14, align 4
  store float %extract17, float addrspace(1)* %17, align 4
  store float %extract18, float addrspace(1)* %20, align 4
  store float %extract19, float addrspace(1)* %23, align 4
  store float %extract20, float addrspace(1)* %26, align 4
  store float %extract21, float addrspace(1)* %29, align 4
  store float %extract22, float addrspace(1)* %32, align 4
  store float %extract23, float addrspace(1)* %35, align 4
  store float %extract24, float addrspace(1)* %38, align 4
  store float %extract25, float addrspace(1)* %41, align 4
  store float %extract26, float addrspace(1)* %44, align 4
  store float %extract27, float addrspace(1)* %47, align 4
  store float %extract28, float addrspace(1)* %50, align 4
  store float %extract29, float addrspace(1)* %53, align 4
  store float %extract30, float addrspace(1)* %56, align 4
  store float %extract31, float addrspace(1)* %59, align 4
  %60 = add nsw <16 x i32> %vectorPHI, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %61 = and <16 x i32> %60, %vector
  %62 = extractelement <16 x i32> %61, i32 0
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds float addrspace(1)* %output, i64 %63
  %65 = extractelement <16 x i32> %61, i32 1
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %output, i64 %66
  %68 = extractelement <16 x i32> %61, i32 2
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds float addrspace(1)* %output, i64 %69
  %71 = extractelement <16 x i32> %61, i32 3
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds float addrspace(1)* %output, i64 %72
  %74 = extractelement <16 x i32> %61, i32 4
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds float addrspace(1)* %output, i64 %75
  %77 = extractelement <16 x i32> %61, i32 5
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds float addrspace(1)* %output, i64 %78
  %80 = extractelement <16 x i32> %61, i32 6
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds float addrspace(1)* %output, i64 %81
  %83 = extractelement <16 x i32> %61, i32 7
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds float addrspace(1)* %output, i64 %84
  %86 = extractelement <16 x i32> %61, i32 8
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds float addrspace(1)* %output, i64 %87
  %89 = extractelement <16 x i32> %61, i32 9
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds float addrspace(1)* %output, i64 %90
  %92 = extractelement <16 x i32> %61, i32 10
  %93 = sext i32 %92 to i64
  %94 = getelementptr inbounds float addrspace(1)* %output, i64 %93
  %95 = extractelement <16 x i32> %61, i32 11
  %96 = sext i32 %95 to i64
  %97 = getelementptr inbounds float addrspace(1)* %output, i64 %96
  %98 = extractelement <16 x i32> %61, i32 12
  %99 = sext i32 %98 to i64
  %100 = getelementptr inbounds float addrspace(1)* %output, i64 %99
  %101 = extractelement <16 x i32> %61, i32 13
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds float addrspace(1)* %output, i64 %102
  %104 = extractelement <16 x i32> %61, i32 14
  %105 = sext i32 %104 to i64
  %106 = getelementptr inbounds float addrspace(1)* %output, i64 %105
  %107 = extractelement <16 x i32> %61, i32 15
  %108 = sext i32 %107 to i64
  %109 = getelementptr inbounds float addrspace(1)* %output, i64 %108
  store float %extract16, float addrspace(1)* %64, align 4
  store float %extract17, float addrspace(1)* %67, align 4
  store float %extract18, float addrspace(1)* %70, align 4
  store float %extract19, float addrspace(1)* %73, align 4
  store float %extract20, float addrspace(1)* %76, align 4
  store float %extract21, float addrspace(1)* %79, align 4
  store float %extract22, float addrspace(1)* %82, align 4
  store float %extract23, float addrspace(1)* %85, align 4
  store float %extract24, float addrspace(1)* %88, align 4
  store float %extract25, float addrspace(1)* %91, align 4
  store float %extract26, float addrspace(1)* %94, align 4
  store float %extract27, float addrspace(1)* %97, align 4
  store float %extract28, float addrspace(1)* %100, align 4
  store float %extract29, float addrspace(1)* %103, align 4
  store float %extract30, float addrspace(1)* %106, align 4
  store float %extract31, float addrspace(1)* %109, align 4
  %110 = add nsw <16 x i32> %vectorPHI, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %111 = and <16 x i32> %110, %vector
  %112 = extractelement <16 x i32> %111, i32 0
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds float addrspace(1)* %output, i64 %113
  %115 = extractelement <16 x i32> %111, i32 1
  %116 = sext i32 %115 to i64
  %117 = getelementptr inbounds float addrspace(1)* %output, i64 %116
  %118 = extractelement <16 x i32> %111, i32 2
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds float addrspace(1)* %output, i64 %119
  %121 = extractelement <16 x i32> %111, i32 3
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds float addrspace(1)* %output, i64 %122
  %124 = extractelement <16 x i32> %111, i32 4
  %125 = sext i32 %124 to i64
  %126 = getelementptr inbounds float addrspace(1)* %output, i64 %125
  %127 = extractelement <16 x i32> %111, i32 5
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds float addrspace(1)* %output, i64 %128
  %130 = extractelement <16 x i32> %111, i32 6
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds float addrspace(1)* %output, i64 %131
  %133 = extractelement <16 x i32> %111, i32 7
  %134 = sext i32 %133 to i64
  %135 = getelementptr inbounds float addrspace(1)* %output, i64 %134
  %136 = extractelement <16 x i32> %111, i32 8
  %137 = sext i32 %136 to i64
  %138 = getelementptr inbounds float addrspace(1)* %output, i64 %137
  %139 = extractelement <16 x i32> %111, i32 9
  %140 = sext i32 %139 to i64
  %141 = getelementptr inbounds float addrspace(1)* %output, i64 %140
  %142 = extractelement <16 x i32> %111, i32 10
  %143 = sext i32 %142 to i64
  %144 = getelementptr inbounds float addrspace(1)* %output, i64 %143
  %145 = extractelement <16 x i32> %111, i32 11
  %146 = sext i32 %145 to i64
  %147 = getelementptr inbounds float addrspace(1)* %output, i64 %146
  %148 = extractelement <16 x i32> %111, i32 12
  %149 = sext i32 %148 to i64
  %150 = getelementptr inbounds float addrspace(1)* %output, i64 %149
  %151 = extractelement <16 x i32> %111, i32 13
  %152 = sext i32 %151 to i64
  %153 = getelementptr inbounds float addrspace(1)* %output, i64 %152
  %154 = extractelement <16 x i32> %111, i32 14
  %155 = sext i32 %154 to i64
  %156 = getelementptr inbounds float addrspace(1)* %output, i64 %155
  %157 = extractelement <16 x i32> %111, i32 15
  %158 = sext i32 %157 to i64
  %159 = getelementptr inbounds float addrspace(1)* %output, i64 %158
  store float %extract16, float addrspace(1)* %114, align 4
  store float %extract17, float addrspace(1)* %117, align 4
  store float %extract18, float addrspace(1)* %120, align 4
  store float %extract19, float addrspace(1)* %123, align 4
  store float %extract20, float addrspace(1)* %126, align 4
  store float %extract21, float addrspace(1)* %129, align 4
  store float %extract22, float addrspace(1)* %132, align 4
  store float %extract23, float addrspace(1)* %135, align 4
  store float %extract24, float addrspace(1)* %138, align 4
  store float %extract25, float addrspace(1)* %141, align 4
  store float %extract26, float addrspace(1)* %144, align 4
  store float %extract27, float addrspace(1)* %147, align 4
  store float %extract28, float addrspace(1)* %150, align 4
  store float %extract29, float addrspace(1)* %153, align 4
  store float %extract30, float addrspace(1)* %156, align 4
  store float %extract31, float addrspace(1)* %159, align 4
  %160 = add nsw <16 x i32> %vectorPHI, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %161 = and <16 x i32> %160, %vector
  %162 = extractelement <16 x i32> %161, i32 0
  %163 = sext i32 %162 to i64
  %164 = getelementptr inbounds float addrspace(1)* %output, i64 %163
  %165 = extractelement <16 x i32> %161, i32 1
  %166 = sext i32 %165 to i64
  %167 = getelementptr inbounds float addrspace(1)* %output, i64 %166
  %168 = extractelement <16 x i32> %161, i32 2
  %169 = sext i32 %168 to i64
  %170 = getelementptr inbounds float addrspace(1)* %output, i64 %169
  %171 = extractelement <16 x i32> %161, i32 3
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds float addrspace(1)* %output, i64 %172
  %174 = extractelement <16 x i32> %161, i32 4
  %175 = sext i32 %174 to i64
  %176 = getelementptr inbounds float addrspace(1)* %output, i64 %175
  %177 = extractelement <16 x i32> %161, i32 5
  %178 = sext i32 %177 to i64
  %179 = getelementptr inbounds float addrspace(1)* %output, i64 %178
  %180 = extractelement <16 x i32> %161, i32 6
  %181 = sext i32 %180 to i64
  %182 = getelementptr inbounds float addrspace(1)* %output, i64 %181
  %183 = extractelement <16 x i32> %161, i32 7
  %184 = sext i32 %183 to i64
  %185 = getelementptr inbounds float addrspace(1)* %output, i64 %184
  %186 = extractelement <16 x i32> %161, i32 8
  %187 = sext i32 %186 to i64
  %188 = getelementptr inbounds float addrspace(1)* %output, i64 %187
  %189 = extractelement <16 x i32> %161, i32 9
  %190 = sext i32 %189 to i64
  %191 = getelementptr inbounds float addrspace(1)* %output, i64 %190
  %192 = extractelement <16 x i32> %161, i32 10
  %193 = sext i32 %192 to i64
  %194 = getelementptr inbounds float addrspace(1)* %output, i64 %193
  %195 = extractelement <16 x i32> %161, i32 11
  %196 = sext i32 %195 to i64
  %197 = getelementptr inbounds float addrspace(1)* %output, i64 %196
  %198 = extractelement <16 x i32> %161, i32 12
  %199 = sext i32 %198 to i64
  %200 = getelementptr inbounds float addrspace(1)* %output, i64 %199
  %201 = extractelement <16 x i32> %161, i32 13
  %202 = sext i32 %201 to i64
  %203 = getelementptr inbounds float addrspace(1)* %output, i64 %202
  %204 = extractelement <16 x i32> %161, i32 14
  %205 = sext i32 %204 to i64
  %206 = getelementptr inbounds float addrspace(1)* %output, i64 %205
  %207 = extractelement <16 x i32> %161, i32 15
  %208 = sext i32 %207 to i64
  %209 = getelementptr inbounds float addrspace(1)* %output, i64 %208
  store float %extract16, float addrspace(1)* %164, align 4
  store float %extract17, float addrspace(1)* %167, align 4
  store float %extract18, float addrspace(1)* %170, align 4
  store float %extract19, float addrspace(1)* %173, align 4
  store float %extract20, float addrspace(1)* %176, align 4
  store float %extract21, float addrspace(1)* %179, align 4
  store float %extract22, float addrspace(1)* %182, align 4
  store float %extract23, float addrspace(1)* %185, align 4
  store float %extract24, float addrspace(1)* %188, align 4
  store float %extract25, float addrspace(1)* %191, align 4
  store float %extract26, float addrspace(1)* %194, align 4
  store float %extract27, float addrspace(1)* %197, align 4
  store float %extract28, float addrspace(1)* %200, align 4
  store float %extract29, float addrspace(1)* %203, align 4
  store float %extract30, float addrspace(1)* %206, align 4
  store float %extract31, float addrspace(1)* %209, align 4
  %210 = add nsw <16 x i32> %vectorPHI, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %211 = and <16 x i32> %210, %vector
  %212 = extractelement <16 x i32> %211, i32 0
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds float addrspace(1)* %output, i64 %213
  %215 = extractelement <16 x i32> %211, i32 1
  %216 = sext i32 %215 to i64
  %217 = getelementptr inbounds float addrspace(1)* %output, i64 %216
  %218 = extractelement <16 x i32> %211, i32 2
  %219 = sext i32 %218 to i64
  %220 = getelementptr inbounds float addrspace(1)* %output, i64 %219
  %221 = extractelement <16 x i32> %211, i32 3
  %222 = sext i32 %221 to i64
  %223 = getelementptr inbounds float addrspace(1)* %output, i64 %222
  %224 = extractelement <16 x i32> %211, i32 4
  %225 = sext i32 %224 to i64
  %226 = getelementptr inbounds float addrspace(1)* %output, i64 %225
  %227 = extractelement <16 x i32> %211, i32 5
  %228 = sext i32 %227 to i64
  %229 = getelementptr inbounds float addrspace(1)* %output, i64 %228
  %230 = extractelement <16 x i32> %211, i32 6
  %231 = sext i32 %230 to i64
  %232 = getelementptr inbounds float addrspace(1)* %output, i64 %231
  %233 = extractelement <16 x i32> %211, i32 7
  %234 = sext i32 %233 to i64
  %235 = getelementptr inbounds float addrspace(1)* %output, i64 %234
  %236 = extractelement <16 x i32> %211, i32 8
  %237 = sext i32 %236 to i64
  %238 = getelementptr inbounds float addrspace(1)* %output, i64 %237
  %239 = extractelement <16 x i32> %211, i32 9
  %240 = sext i32 %239 to i64
  %241 = getelementptr inbounds float addrspace(1)* %output, i64 %240
  %242 = extractelement <16 x i32> %211, i32 10
  %243 = sext i32 %242 to i64
  %244 = getelementptr inbounds float addrspace(1)* %output, i64 %243
  %245 = extractelement <16 x i32> %211, i32 11
  %246 = sext i32 %245 to i64
  %247 = getelementptr inbounds float addrspace(1)* %output, i64 %246
  %248 = extractelement <16 x i32> %211, i32 12
  %249 = sext i32 %248 to i64
  %250 = getelementptr inbounds float addrspace(1)* %output, i64 %249
  %251 = extractelement <16 x i32> %211, i32 13
  %252 = sext i32 %251 to i64
  %253 = getelementptr inbounds float addrspace(1)* %output, i64 %252
  %254 = extractelement <16 x i32> %211, i32 14
  %255 = sext i32 %254 to i64
  %256 = getelementptr inbounds float addrspace(1)* %output, i64 %255
  %257 = extractelement <16 x i32> %211, i32 15
  %258 = sext i32 %257 to i64
  %259 = getelementptr inbounds float addrspace(1)* %output, i64 %258
  store float %extract16, float addrspace(1)* %214, align 4
  store float %extract17, float addrspace(1)* %217, align 4
  store float %extract18, float addrspace(1)* %220, align 4
  store float %extract19, float addrspace(1)* %223, align 4
  store float %extract20, float addrspace(1)* %226, align 4
  store float %extract21, float addrspace(1)* %229, align 4
  store float %extract22, float addrspace(1)* %232, align 4
  store float %extract23, float addrspace(1)* %235, align 4
  store float %extract24, float addrspace(1)* %238, align 4
  store float %extract25, float addrspace(1)* %241, align 4
  store float %extract26, float addrspace(1)* %244, align 4
  store float %extract27, float addrspace(1)* %247, align 4
  store float %extract28, float addrspace(1)* %250, align 4
  store float %extract29, float addrspace(1)* %253, align 4
  store float %extract30, float addrspace(1)* %256, align 4
  store float %extract31, float addrspace(1)* %259, align 4
  %260 = add nsw <16 x i32> %vectorPHI, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %261 = and <16 x i32> %260, %vector
  %262 = extractelement <16 x i32> %261, i32 0
  %263 = sext i32 %262 to i64
  %264 = getelementptr inbounds float addrspace(1)* %output, i64 %263
  %265 = extractelement <16 x i32> %261, i32 1
  %266 = sext i32 %265 to i64
  %267 = getelementptr inbounds float addrspace(1)* %output, i64 %266
  %268 = extractelement <16 x i32> %261, i32 2
  %269 = sext i32 %268 to i64
  %270 = getelementptr inbounds float addrspace(1)* %output, i64 %269
  %271 = extractelement <16 x i32> %261, i32 3
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds float addrspace(1)* %output, i64 %272
  %274 = extractelement <16 x i32> %261, i32 4
  %275 = sext i32 %274 to i64
  %276 = getelementptr inbounds float addrspace(1)* %output, i64 %275
  %277 = extractelement <16 x i32> %261, i32 5
  %278 = sext i32 %277 to i64
  %279 = getelementptr inbounds float addrspace(1)* %output, i64 %278
  %280 = extractelement <16 x i32> %261, i32 6
  %281 = sext i32 %280 to i64
  %282 = getelementptr inbounds float addrspace(1)* %output, i64 %281
  %283 = extractelement <16 x i32> %261, i32 7
  %284 = sext i32 %283 to i64
  %285 = getelementptr inbounds float addrspace(1)* %output, i64 %284
  %286 = extractelement <16 x i32> %261, i32 8
  %287 = sext i32 %286 to i64
  %288 = getelementptr inbounds float addrspace(1)* %output, i64 %287
  %289 = extractelement <16 x i32> %261, i32 9
  %290 = sext i32 %289 to i64
  %291 = getelementptr inbounds float addrspace(1)* %output, i64 %290
  %292 = extractelement <16 x i32> %261, i32 10
  %293 = sext i32 %292 to i64
  %294 = getelementptr inbounds float addrspace(1)* %output, i64 %293
  %295 = extractelement <16 x i32> %261, i32 11
  %296 = sext i32 %295 to i64
  %297 = getelementptr inbounds float addrspace(1)* %output, i64 %296
  %298 = extractelement <16 x i32> %261, i32 12
  %299 = sext i32 %298 to i64
  %300 = getelementptr inbounds float addrspace(1)* %output, i64 %299
  %301 = extractelement <16 x i32> %261, i32 13
  %302 = sext i32 %301 to i64
  %303 = getelementptr inbounds float addrspace(1)* %output, i64 %302
  %304 = extractelement <16 x i32> %261, i32 14
  %305 = sext i32 %304 to i64
  %306 = getelementptr inbounds float addrspace(1)* %output, i64 %305
  %307 = extractelement <16 x i32> %261, i32 15
  %308 = sext i32 %307 to i64
  %309 = getelementptr inbounds float addrspace(1)* %output, i64 %308
  store float %extract16, float addrspace(1)* %264, align 4
  store float %extract17, float addrspace(1)* %267, align 4
  store float %extract18, float addrspace(1)* %270, align 4
  store float %extract19, float addrspace(1)* %273, align 4
  store float %extract20, float addrspace(1)* %276, align 4
  store float %extract21, float addrspace(1)* %279, align 4
  store float %extract22, float addrspace(1)* %282, align 4
  store float %extract23, float addrspace(1)* %285, align 4
  store float %extract24, float addrspace(1)* %288, align 4
  store float %extract25, float addrspace(1)* %291, align 4
  store float %extract26, float addrspace(1)* %294, align 4
  store float %extract27, float addrspace(1)* %297, align 4
  store float %extract28, float addrspace(1)* %300, align 4
  store float %extract29, float addrspace(1)* %303, align 4
  store float %extract30, float addrspace(1)* %306, align 4
  store float %extract31, float addrspace(1)* %309, align 4
  %310 = add nsw <16 x i32> %vectorPHI, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %311 = and <16 x i32> %310, %vector
  %312 = extractelement <16 x i32> %311, i32 0
  %313 = sext i32 %312 to i64
  %314 = getelementptr inbounds float addrspace(1)* %output, i64 %313
  %315 = extractelement <16 x i32> %311, i32 1
  %316 = sext i32 %315 to i64
  %317 = getelementptr inbounds float addrspace(1)* %output, i64 %316
  %318 = extractelement <16 x i32> %311, i32 2
  %319 = sext i32 %318 to i64
  %320 = getelementptr inbounds float addrspace(1)* %output, i64 %319
  %321 = extractelement <16 x i32> %311, i32 3
  %322 = sext i32 %321 to i64
  %323 = getelementptr inbounds float addrspace(1)* %output, i64 %322
  %324 = extractelement <16 x i32> %311, i32 4
  %325 = sext i32 %324 to i64
  %326 = getelementptr inbounds float addrspace(1)* %output, i64 %325
  %327 = extractelement <16 x i32> %311, i32 5
  %328 = sext i32 %327 to i64
  %329 = getelementptr inbounds float addrspace(1)* %output, i64 %328
  %330 = extractelement <16 x i32> %311, i32 6
  %331 = sext i32 %330 to i64
  %332 = getelementptr inbounds float addrspace(1)* %output, i64 %331
  %333 = extractelement <16 x i32> %311, i32 7
  %334 = sext i32 %333 to i64
  %335 = getelementptr inbounds float addrspace(1)* %output, i64 %334
  %336 = extractelement <16 x i32> %311, i32 8
  %337 = sext i32 %336 to i64
  %338 = getelementptr inbounds float addrspace(1)* %output, i64 %337
  %339 = extractelement <16 x i32> %311, i32 9
  %340 = sext i32 %339 to i64
  %341 = getelementptr inbounds float addrspace(1)* %output, i64 %340
  %342 = extractelement <16 x i32> %311, i32 10
  %343 = sext i32 %342 to i64
  %344 = getelementptr inbounds float addrspace(1)* %output, i64 %343
  %345 = extractelement <16 x i32> %311, i32 11
  %346 = sext i32 %345 to i64
  %347 = getelementptr inbounds float addrspace(1)* %output, i64 %346
  %348 = extractelement <16 x i32> %311, i32 12
  %349 = sext i32 %348 to i64
  %350 = getelementptr inbounds float addrspace(1)* %output, i64 %349
  %351 = extractelement <16 x i32> %311, i32 13
  %352 = sext i32 %351 to i64
  %353 = getelementptr inbounds float addrspace(1)* %output, i64 %352
  %354 = extractelement <16 x i32> %311, i32 14
  %355 = sext i32 %354 to i64
  %356 = getelementptr inbounds float addrspace(1)* %output, i64 %355
  %357 = extractelement <16 x i32> %311, i32 15
  %358 = sext i32 %357 to i64
  %359 = getelementptr inbounds float addrspace(1)* %output, i64 %358
  store float %extract16, float addrspace(1)* %314, align 4
  store float %extract17, float addrspace(1)* %317, align 4
  store float %extract18, float addrspace(1)* %320, align 4
  store float %extract19, float addrspace(1)* %323, align 4
  store float %extract20, float addrspace(1)* %326, align 4
  store float %extract21, float addrspace(1)* %329, align 4
  store float %extract22, float addrspace(1)* %332, align 4
  store float %extract23, float addrspace(1)* %335, align 4
  store float %extract24, float addrspace(1)* %338, align 4
  store float %extract25, float addrspace(1)* %341, align 4
  store float %extract26, float addrspace(1)* %344, align 4
  store float %extract27, float addrspace(1)* %347, align 4
  store float %extract28, float addrspace(1)* %350, align 4
  store float %extract29, float addrspace(1)* %353, align 4
  store float %extract30, float addrspace(1)* %356, align 4
  store float %extract31, float addrspace(1)* %359, align 4
  %360 = add nsw <16 x i32> %vectorPHI, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %361 = and <16 x i32> %360, %vector
  %362 = extractelement <16 x i32> %361, i32 0
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds float addrspace(1)* %output, i64 %363
  %365 = extractelement <16 x i32> %361, i32 1
  %366 = sext i32 %365 to i64
  %367 = getelementptr inbounds float addrspace(1)* %output, i64 %366
  %368 = extractelement <16 x i32> %361, i32 2
  %369 = sext i32 %368 to i64
  %370 = getelementptr inbounds float addrspace(1)* %output, i64 %369
  %371 = extractelement <16 x i32> %361, i32 3
  %372 = sext i32 %371 to i64
  %373 = getelementptr inbounds float addrspace(1)* %output, i64 %372
  %374 = extractelement <16 x i32> %361, i32 4
  %375 = sext i32 %374 to i64
  %376 = getelementptr inbounds float addrspace(1)* %output, i64 %375
  %377 = extractelement <16 x i32> %361, i32 5
  %378 = sext i32 %377 to i64
  %379 = getelementptr inbounds float addrspace(1)* %output, i64 %378
  %380 = extractelement <16 x i32> %361, i32 6
  %381 = sext i32 %380 to i64
  %382 = getelementptr inbounds float addrspace(1)* %output, i64 %381
  %383 = extractelement <16 x i32> %361, i32 7
  %384 = sext i32 %383 to i64
  %385 = getelementptr inbounds float addrspace(1)* %output, i64 %384
  %386 = extractelement <16 x i32> %361, i32 8
  %387 = sext i32 %386 to i64
  %388 = getelementptr inbounds float addrspace(1)* %output, i64 %387
  %389 = extractelement <16 x i32> %361, i32 9
  %390 = sext i32 %389 to i64
  %391 = getelementptr inbounds float addrspace(1)* %output, i64 %390
  %392 = extractelement <16 x i32> %361, i32 10
  %393 = sext i32 %392 to i64
  %394 = getelementptr inbounds float addrspace(1)* %output, i64 %393
  %395 = extractelement <16 x i32> %361, i32 11
  %396 = sext i32 %395 to i64
  %397 = getelementptr inbounds float addrspace(1)* %output, i64 %396
  %398 = extractelement <16 x i32> %361, i32 12
  %399 = sext i32 %398 to i64
  %400 = getelementptr inbounds float addrspace(1)* %output, i64 %399
  %401 = extractelement <16 x i32> %361, i32 13
  %402 = sext i32 %401 to i64
  %403 = getelementptr inbounds float addrspace(1)* %output, i64 %402
  %404 = extractelement <16 x i32> %361, i32 14
  %405 = sext i32 %404 to i64
  %406 = getelementptr inbounds float addrspace(1)* %output, i64 %405
  %407 = extractelement <16 x i32> %361, i32 15
  %408 = sext i32 %407 to i64
  %409 = getelementptr inbounds float addrspace(1)* %output, i64 %408
  store float %extract16, float addrspace(1)* %364, align 4
  store float %extract17, float addrspace(1)* %367, align 4
  store float %extract18, float addrspace(1)* %370, align 4
  store float %extract19, float addrspace(1)* %373, align 4
  store float %extract20, float addrspace(1)* %376, align 4
  store float %extract21, float addrspace(1)* %379, align 4
  store float %extract22, float addrspace(1)* %382, align 4
  store float %extract23, float addrspace(1)* %385, align 4
  store float %extract24, float addrspace(1)* %388, align 4
  store float %extract25, float addrspace(1)* %391, align 4
  store float %extract26, float addrspace(1)* %394, align 4
  store float %extract27, float addrspace(1)* %397, align 4
  store float %extract28, float addrspace(1)* %400, align 4
  store float %extract29, float addrspace(1)* %403, align 4
  store float %extract30, float addrspace(1)* %406, align 4
  store float %extract31, float addrspace(1)* %409, align 4
  %410 = add nsw <16 x i32> %vectorPHI, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %411 = and <16 x i32> %410, %vector
  %412 = extractelement <16 x i32> %411, i32 0
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds float addrspace(1)* %output, i64 %413
  %415 = extractelement <16 x i32> %411, i32 1
  %416 = sext i32 %415 to i64
  %417 = getelementptr inbounds float addrspace(1)* %output, i64 %416
  %418 = extractelement <16 x i32> %411, i32 2
  %419 = sext i32 %418 to i64
  %420 = getelementptr inbounds float addrspace(1)* %output, i64 %419
  %421 = extractelement <16 x i32> %411, i32 3
  %422 = sext i32 %421 to i64
  %423 = getelementptr inbounds float addrspace(1)* %output, i64 %422
  %424 = extractelement <16 x i32> %411, i32 4
  %425 = sext i32 %424 to i64
  %426 = getelementptr inbounds float addrspace(1)* %output, i64 %425
  %427 = extractelement <16 x i32> %411, i32 5
  %428 = sext i32 %427 to i64
  %429 = getelementptr inbounds float addrspace(1)* %output, i64 %428
  %430 = extractelement <16 x i32> %411, i32 6
  %431 = sext i32 %430 to i64
  %432 = getelementptr inbounds float addrspace(1)* %output, i64 %431
  %433 = extractelement <16 x i32> %411, i32 7
  %434 = sext i32 %433 to i64
  %435 = getelementptr inbounds float addrspace(1)* %output, i64 %434
  %436 = extractelement <16 x i32> %411, i32 8
  %437 = sext i32 %436 to i64
  %438 = getelementptr inbounds float addrspace(1)* %output, i64 %437
  %439 = extractelement <16 x i32> %411, i32 9
  %440 = sext i32 %439 to i64
  %441 = getelementptr inbounds float addrspace(1)* %output, i64 %440
  %442 = extractelement <16 x i32> %411, i32 10
  %443 = sext i32 %442 to i64
  %444 = getelementptr inbounds float addrspace(1)* %output, i64 %443
  %445 = extractelement <16 x i32> %411, i32 11
  %446 = sext i32 %445 to i64
  %447 = getelementptr inbounds float addrspace(1)* %output, i64 %446
  %448 = extractelement <16 x i32> %411, i32 12
  %449 = sext i32 %448 to i64
  %450 = getelementptr inbounds float addrspace(1)* %output, i64 %449
  %451 = extractelement <16 x i32> %411, i32 13
  %452 = sext i32 %451 to i64
  %453 = getelementptr inbounds float addrspace(1)* %output, i64 %452
  %454 = extractelement <16 x i32> %411, i32 14
  %455 = sext i32 %454 to i64
  %456 = getelementptr inbounds float addrspace(1)* %output, i64 %455
  %457 = extractelement <16 x i32> %411, i32 15
  %458 = sext i32 %457 to i64
  %459 = getelementptr inbounds float addrspace(1)* %output, i64 %458
  store float %extract16, float addrspace(1)* %414, align 4
  store float %extract17, float addrspace(1)* %417, align 4
  store float %extract18, float addrspace(1)* %420, align 4
  store float %extract19, float addrspace(1)* %423, align 4
  store float %extract20, float addrspace(1)* %426, align 4
  store float %extract21, float addrspace(1)* %429, align 4
  store float %extract22, float addrspace(1)* %432, align 4
  store float %extract23, float addrspace(1)* %435, align 4
  store float %extract24, float addrspace(1)* %438, align 4
  store float %extract25, float addrspace(1)* %441, align 4
  store float %extract26, float addrspace(1)* %444, align 4
  store float %extract27, float addrspace(1)* %447, align 4
  store float %extract28, float addrspace(1)* %450, align 4
  store float %extract29, float addrspace(1)* %453, align 4
  store float %extract30, float addrspace(1)* %456, align 4
  store float %extract31, float addrspace(1)* %459, align 4
  %460 = add nsw <16 x i32> %vectorPHI, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %461 = and <16 x i32> %460, %vector
  %462 = extractelement <16 x i32> %461, i32 0
  %463 = sext i32 %462 to i64
  %464 = getelementptr inbounds float addrspace(1)* %output, i64 %463
  %465 = extractelement <16 x i32> %461, i32 1
  %466 = sext i32 %465 to i64
  %467 = getelementptr inbounds float addrspace(1)* %output, i64 %466
  %468 = extractelement <16 x i32> %461, i32 2
  %469 = sext i32 %468 to i64
  %470 = getelementptr inbounds float addrspace(1)* %output, i64 %469
  %471 = extractelement <16 x i32> %461, i32 3
  %472 = sext i32 %471 to i64
  %473 = getelementptr inbounds float addrspace(1)* %output, i64 %472
  %474 = extractelement <16 x i32> %461, i32 4
  %475 = sext i32 %474 to i64
  %476 = getelementptr inbounds float addrspace(1)* %output, i64 %475
  %477 = extractelement <16 x i32> %461, i32 5
  %478 = sext i32 %477 to i64
  %479 = getelementptr inbounds float addrspace(1)* %output, i64 %478
  %480 = extractelement <16 x i32> %461, i32 6
  %481 = sext i32 %480 to i64
  %482 = getelementptr inbounds float addrspace(1)* %output, i64 %481
  %483 = extractelement <16 x i32> %461, i32 7
  %484 = sext i32 %483 to i64
  %485 = getelementptr inbounds float addrspace(1)* %output, i64 %484
  %486 = extractelement <16 x i32> %461, i32 8
  %487 = sext i32 %486 to i64
  %488 = getelementptr inbounds float addrspace(1)* %output, i64 %487
  %489 = extractelement <16 x i32> %461, i32 9
  %490 = sext i32 %489 to i64
  %491 = getelementptr inbounds float addrspace(1)* %output, i64 %490
  %492 = extractelement <16 x i32> %461, i32 10
  %493 = sext i32 %492 to i64
  %494 = getelementptr inbounds float addrspace(1)* %output, i64 %493
  %495 = extractelement <16 x i32> %461, i32 11
  %496 = sext i32 %495 to i64
  %497 = getelementptr inbounds float addrspace(1)* %output, i64 %496
  %498 = extractelement <16 x i32> %461, i32 12
  %499 = sext i32 %498 to i64
  %500 = getelementptr inbounds float addrspace(1)* %output, i64 %499
  %501 = extractelement <16 x i32> %461, i32 13
  %502 = sext i32 %501 to i64
  %503 = getelementptr inbounds float addrspace(1)* %output, i64 %502
  %504 = extractelement <16 x i32> %461, i32 14
  %505 = sext i32 %504 to i64
  %506 = getelementptr inbounds float addrspace(1)* %output, i64 %505
  %507 = extractelement <16 x i32> %461, i32 15
  %508 = sext i32 %507 to i64
  %509 = getelementptr inbounds float addrspace(1)* %output, i64 %508
  store float %extract16, float addrspace(1)* %464, align 4
  store float %extract17, float addrspace(1)* %467, align 4
  store float %extract18, float addrspace(1)* %470, align 4
  store float %extract19, float addrspace(1)* %473, align 4
  store float %extract20, float addrspace(1)* %476, align 4
  store float %extract21, float addrspace(1)* %479, align 4
  store float %extract22, float addrspace(1)* %482, align 4
  store float %extract23, float addrspace(1)* %485, align 4
  store float %extract24, float addrspace(1)* %488, align 4
  store float %extract25, float addrspace(1)* %491, align 4
  store float %extract26, float addrspace(1)* %494, align 4
  store float %extract27, float addrspace(1)* %497, align 4
  store float %extract28, float addrspace(1)* %500, align 4
  store float %extract29, float addrspace(1)* %503, align 4
  store float %extract30, float addrspace(1)* %506, align 4
  store float %extract31, float addrspace(1)* %509, align 4
  %510 = add nsw <16 x i32> %vectorPHI, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %511 = and <16 x i32> %510, %vector
  %512 = extractelement <16 x i32> %511, i32 0
  %513 = sext i32 %512 to i64
  %514 = getelementptr inbounds float addrspace(1)* %output, i64 %513
  %515 = extractelement <16 x i32> %511, i32 1
  %516 = sext i32 %515 to i64
  %517 = getelementptr inbounds float addrspace(1)* %output, i64 %516
  %518 = extractelement <16 x i32> %511, i32 2
  %519 = sext i32 %518 to i64
  %520 = getelementptr inbounds float addrspace(1)* %output, i64 %519
  %521 = extractelement <16 x i32> %511, i32 3
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds float addrspace(1)* %output, i64 %522
  %524 = extractelement <16 x i32> %511, i32 4
  %525 = sext i32 %524 to i64
  %526 = getelementptr inbounds float addrspace(1)* %output, i64 %525
  %527 = extractelement <16 x i32> %511, i32 5
  %528 = sext i32 %527 to i64
  %529 = getelementptr inbounds float addrspace(1)* %output, i64 %528
  %530 = extractelement <16 x i32> %511, i32 6
  %531 = sext i32 %530 to i64
  %532 = getelementptr inbounds float addrspace(1)* %output, i64 %531
  %533 = extractelement <16 x i32> %511, i32 7
  %534 = sext i32 %533 to i64
  %535 = getelementptr inbounds float addrspace(1)* %output, i64 %534
  %536 = extractelement <16 x i32> %511, i32 8
  %537 = sext i32 %536 to i64
  %538 = getelementptr inbounds float addrspace(1)* %output, i64 %537
  %539 = extractelement <16 x i32> %511, i32 9
  %540 = sext i32 %539 to i64
  %541 = getelementptr inbounds float addrspace(1)* %output, i64 %540
  %542 = extractelement <16 x i32> %511, i32 10
  %543 = sext i32 %542 to i64
  %544 = getelementptr inbounds float addrspace(1)* %output, i64 %543
  %545 = extractelement <16 x i32> %511, i32 11
  %546 = sext i32 %545 to i64
  %547 = getelementptr inbounds float addrspace(1)* %output, i64 %546
  %548 = extractelement <16 x i32> %511, i32 12
  %549 = sext i32 %548 to i64
  %550 = getelementptr inbounds float addrspace(1)* %output, i64 %549
  %551 = extractelement <16 x i32> %511, i32 13
  %552 = sext i32 %551 to i64
  %553 = getelementptr inbounds float addrspace(1)* %output, i64 %552
  %554 = extractelement <16 x i32> %511, i32 14
  %555 = sext i32 %554 to i64
  %556 = getelementptr inbounds float addrspace(1)* %output, i64 %555
  %557 = extractelement <16 x i32> %511, i32 15
  %558 = sext i32 %557 to i64
  %559 = getelementptr inbounds float addrspace(1)* %output, i64 %558
  store float %extract16, float addrspace(1)* %514, align 4
  store float %extract17, float addrspace(1)* %517, align 4
  store float %extract18, float addrspace(1)* %520, align 4
  store float %extract19, float addrspace(1)* %523, align 4
  store float %extract20, float addrspace(1)* %526, align 4
  store float %extract21, float addrspace(1)* %529, align 4
  store float %extract22, float addrspace(1)* %532, align 4
  store float %extract23, float addrspace(1)* %535, align 4
  store float %extract24, float addrspace(1)* %538, align 4
  store float %extract25, float addrspace(1)* %541, align 4
  store float %extract26, float addrspace(1)* %544, align 4
  store float %extract27, float addrspace(1)* %547, align 4
  store float %extract28, float addrspace(1)* %550, align 4
  store float %extract29, float addrspace(1)* %553, align 4
  store float %extract30, float addrspace(1)* %556, align 4
  store float %extract31, float addrspace(1)* %559, align 4
  %560 = add nsw <16 x i32> %vectorPHI, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %561 = and <16 x i32> %560, %vector
  %562 = extractelement <16 x i32> %561, i32 0
  %563 = sext i32 %562 to i64
  %564 = getelementptr inbounds float addrspace(1)* %output, i64 %563
  %565 = extractelement <16 x i32> %561, i32 1
  %566 = sext i32 %565 to i64
  %567 = getelementptr inbounds float addrspace(1)* %output, i64 %566
  %568 = extractelement <16 x i32> %561, i32 2
  %569 = sext i32 %568 to i64
  %570 = getelementptr inbounds float addrspace(1)* %output, i64 %569
  %571 = extractelement <16 x i32> %561, i32 3
  %572 = sext i32 %571 to i64
  %573 = getelementptr inbounds float addrspace(1)* %output, i64 %572
  %574 = extractelement <16 x i32> %561, i32 4
  %575 = sext i32 %574 to i64
  %576 = getelementptr inbounds float addrspace(1)* %output, i64 %575
  %577 = extractelement <16 x i32> %561, i32 5
  %578 = sext i32 %577 to i64
  %579 = getelementptr inbounds float addrspace(1)* %output, i64 %578
  %580 = extractelement <16 x i32> %561, i32 6
  %581 = sext i32 %580 to i64
  %582 = getelementptr inbounds float addrspace(1)* %output, i64 %581
  %583 = extractelement <16 x i32> %561, i32 7
  %584 = sext i32 %583 to i64
  %585 = getelementptr inbounds float addrspace(1)* %output, i64 %584
  %586 = extractelement <16 x i32> %561, i32 8
  %587 = sext i32 %586 to i64
  %588 = getelementptr inbounds float addrspace(1)* %output, i64 %587
  %589 = extractelement <16 x i32> %561, i32 9
  %590 = sext i32 %589 to i64
  %591 = getelementptr inbounds float addrspace(1)* %output, i64 %590
  %592 = extractelement <16 x i32> %561, i32 10
  %593 = sext i32 %592 to i64
  %594 = getelementptr inbounds float addrspace(1)* %output, i64 %593
  %595 = extractelement <16 x i32> %561, i32 11
  %596 = sext i32 %595 to i64
  %597 = getelementptr inbounds float addrspace(1)* %output, i64 %596
  %598 = extractelement <16 x i32> %561, i32 12
  %599 = sext i32 %598 to i64
  %600 = getelementptr inbounds float addrspace(1)* %output, i64 %599
  %601 = extractelement <16 x i32> %561, i32 13
  %602 = sext i32 %601 to i64
  %603 = getelementptr inbounds float addrspace(1)* %output, i64 %602
  %604 = extractelement <16 x i32> %561, i32 14
  %605 = sext i32 %604 to i64
  %606 = getelementptr inbounds float addrspace(1)* %output, i64 %605
  %607 = extractelement <16 x i32> %561, i32 15
  %608 = sext i32 %607 to i64
  %609 = getelementptr inbounds float addrspace(1)* %output, i64 %608
  store float %extract16, float addrspace(1)* %564, align 4
  store float %extract17, float addrspace(1)* %567, align 4
  store float %extract18, float addrspace(1)* %570, align 4
  store float %extract19, float addrspace(1)* %573, align 4
  store float %extract20, float addrspace(1)* %576, align 4
  store float %extract21, float addrspace(1)* %579, align 4
  store float %extract22, float addrspace(1)* %582, align 4
  store float %extract23, float addrspace(1)* %585, align 4
  store float %extract24, float addrspace(1)* %588, align 4
  store float %extract25, float addrspace(1)* %591, align 4
  store float %extract26, float addrspace(1)* %594, align 4
  store float %extract27, float addrspace(1)* %597, align 4
  store float %extract28, float addrspace(1)* %600, align 4
  store float %extract29, float addrspace(1)* %603, align 4
  store float %extract30, float addrspace(1)* %606, align 4
  store float %extract31, float addrspace(1)* %609, align 4
  %610 = add nsw <16 x i32> %vectorPHI, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %611 = and <16 x i32> %610, %vector
  %612 = extractelement <16 x i32> %611, i32 0
  %613 = sext i32 %612 to i64
  %614 = getelementptr inbounds float addrspace(1)* %output, i64 %613
  %615 = extractelement <16 x i32> %611, i32 1
  %616 = sext i32 %615 to i64
  %617 = getelementptr inbounds float addrspace(1)* %output, i64 %616
  %618 = extractelement <16 x i32> %611, i32 2
  %619 = sext i32 %618 to i64
  %620 = getelementptr inbounds float addrspace(1)* %output, i64 %619
  %621 = extractelement <16 x i32> %611, i32 3
  %622 = sext i32 %621 to i64
  %623 = getelementptr inbounds float addrspace(1)* %output, i64 %622
  %624 = extractelement <16 x i32> %611, i32 4
  %625 = sext i32 %624 to i64
  %626 = getelementptr inbounds float addrspace(1)* %output, i64 %625
  %627 = extractelement <16 x i32> %611, i32 5
  %628 = sext i32 %627 to i64
  %629 = getelementptr inbounds float addrspace(1)* %output, i64 %628
  %630 = extractelement <16 x i32> %611, i32 6
  %631 = sext i32 %630 to i64
  %632 = getelementptr inbounds float addrspace(1)* %output, i64 %631
  %633 = extractelement <16 x i32> %611, i32 7
  %634 = sext i32 %633 to i64
  %635 = getelementptr inbounds float addrspace(1)* %output, i64 %634
  %636 = extractelement <16 x i32> %611, i32 8
  %637 = sext i32 %636 to i64
  %638 = getelementptr inbounds float addrspace(1)* %output, i64 %637
  %639 = extractelement <16 x i32> %611, i32 9
  %640 = sext i32 %639 to i64
  %641 = getelementptr inbounds float addrspace(1)* %output, i64 %640
  %642 = extractelement <16 x i32> %611, i32 10
  %643 = sext i32 %642 to i64
  %644 = getelementptr inbounds float addrspace(1)* %output, i64 %643
  %645 = extractelement <16 x i32> %611, i32 11
  %646 = sext i32 %645 to i64
  %647 = getelementptr inbounds float addrspace(1)* %output, i64 %646
  %648 = extractelement <16 x i32> %611, i32 12
  %649 = sext i32 %648 to i64
  %650 = getelementptr inbounds float addrspace(1)* %output, i64 %649
  %651 = extractelement <16 x i32> %611, i32 13
  %652 = sext i32 %651 to i64
  %653 = getelementptr inbounds float addrspace(1)* %output, i64 %652
  %654 = extractelement <16 x i32> %611, i32 14
  %655 = sext i32 %654 to i64
  %656 = getelementptr inbounds float addrspace(1)* %output, i64 %655
  %657 = extractelement <16 x i32> %611, i32 15
  %658 = sext i32 %657 to i64
  %659 = getelementptr inbounds float addrspace(1)* %output, i64 %658
  store float %extract16, float addrspace(1)* %614, align 4
  store float %extract17, float addrspace(1)* %617, align 4
  store float %extract18, float addrspace(1)* %620, align 4
  store float %extract19, float addrspace(1)* %623, align 4
  store float %extract20, float addrspace(1)* %626, align 4
  store float %extract21, float addrspace(1)* %629, align 4
  store float %extract22, float addrspace(1)* %632, align 4
  store float %extract23, float addrspace(1)* %635, align 4
  store float %extract24, float addrspace(1)* %638, align 4
  store float %extract25, float addrspace(1)* %641, align 4
  store float %extract26, float addrspace(1)* %644, align 4
  store float %extract27, float addrspace(1)* %647, align 4
  store float %extract28, float addrspace(1)* %650, align 4
  store float %extract29, float addrspace(1)* %653, align 4
  store float %extract30, float addrspace(1)* %656, align 4
  store float %extract31, float addrspace(1)* %659, align 4
  %660 = add nsw <16 x i32> %vectorPHI, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %661 = and <16 x i32> %660, %vector
  %662 = extractelement <16 x i32> %661, i32 0
  %663 = sext i32 %662 to i64
  %664 = getelementptr inbounds float addrspace(1)* %output, i64 %663
  %665 = extractelement <16 x i32> %661, i32 1
  %666 = sext i32 %665 to i64
  %667 = getelementptr inbounds float addrspace(1)* %output, i64 %666
  %668 = extractelement <16 x i32> %661, i32 2
  %669 = sext i32 %668 to i64
  %670 = getelementptr inbounds float addrspace(1)* %output, i64 %669
  %671 = extractelement <16 x i32> %661, i32 3
  %672 = sext i32 %671 to i64
  %673 = getelementptr inbounds float addrspace(1)* %output, i64 %672
  %674 = extractelement <16 x i32> %661, i32 4
  %675 = sext i32 %674 to i64
  %676 = getelementptr inbounds float addrspace(1)* %output, i64 %675
  %677 = extractelement <16 x i32> %661, i32 5
  %678 = sext i32 %677 to i64
  %679 = getelementptr inbounds float addrspace(1)* %output, i64 %678
  %680 = extractelement <16 x i32> %661, i32 6
  %681 = sext i32 %680 to i64
  %682 = getelementptr inbounds float addrspace(1)* %output, i64 %681
  %683 = extractelement <16 x i32> %661, i32 7
  %684 = sext i32 %683 to i64
  %685 = getelementptr inbounds float addrspace(1)* %output, i64 %684
  %686 = extractelement <16 x i32> %661, i32 8
  %687 = sext i32 %686 to i64
  %688 = getelementptr inbounds float addrspace(1)* %output, i64 %687
  %689 = extractelement <16 x i32> %661, i32 9
  %690 = sext i32 %689 to i64
  %691 = getelementptr inbounds float addrspace(1)* %output, i64 %690
  %692 = extractelement <16 x i32> %661, i32 10
  %693 = sext i32 %692 to i64
  %694 = getelementptr inbounds float addrspace(1)* %output, i64 %693
  %695 = extractelement <16 x i32> %661, i32 11
  %696 = sext i32 %695 to i64
  %697 = getelementptr inbounds float addrspace(1)* %output, i64 %696
  %698 = extractelement <16 x i32> %661, i32 12
  %699 = sext i32 %698 to i64
  %700 = getelementptr inbounds float addrspace(1)* %output, i64 %699
  %701 = extractelement <16 x i32> %661, i32 13
  %702 = sext i32 %701 to i64
  %703 = getelementptr inbounds float addrspace(1)* %output, i64 %702
  %704 = extractelement <16 x i32> %661, i32 14
  %705 = sext i32 %704 to i64
  %706 = getelementptr inbounds float addrspace(1)* %output, i64 %705
  %707 = extractelement <16 x i32> %661, i32 15
  %708 = sext i32 %707 to i64
  %709 = getelementptr inbounds float addrspace(1)* %output, i64 %708
  store float %extract16, float addrspace(1)* %664, align 4
  store float %extract17, float addrspace(1)* %667, align 4
  store float %extract18, float addrspace(1)* %670, align 4
  store float %extract19, float addrspace(1)* %673, align 4
  store float %extract20, float addrspace(1)* %676, align 4
  store float %extract21, float addrspace(1)* %679, align 4
  store float %extract22, float addrspace(1)* %682, align 4
  store float %extract23, float addrspace(1)* %685, align 4
  store float %extract24, float addrspace(1)* %688, align 4
  store float %extract25, float addrspace(1)* %691, align 4
  store float %extract26, float addrspace(1)* %694, align 4
  store float %extract27, float addrspace(1)* %697, align 4
  store float %extract28, float addrspace(1)* %700, align 4
  store float %extract29, float addrspace(1)* %703, align 4
  store float %extract30, float addrspace(1)* %706, align 4
  store float %extract31, float addrspace(1)* %709, align 4
  %710 = add nsw <16 x i32> %vectorPHI, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %711 = and <16 x i32> %710, %vector
  %712 = extractelement <16 x i32> %711, i32 0
  %713 = sext i32 %712 to i64
  %714 = getelementptr inbounds float addrspace(1)* %output, i64 %713
  %715 = extractelement <16 x i32> %711, i32 1
  %716 = sext i32 %715 to i64
  %717 = getelementptr inbounds float addrspace(1)* %output, i64 %716
  %718 = extractelement <16 x i32> %711, i32 2
  %719 = sext i32 %718 to i64
  %720 = getelementptr inbounds float addrspace(1)* %output, i64 %719
  %721 = extractelement <16 x i32> %711, i32 3
  %722 = sext i32 %721 to i64
  %723 = getelementptr inbounds float addrspace(1)* %output, i64 %722
  %724 = extractelement <16 x i32> %711, i32 4
  %725 = sext i32 %724 to i64
  %726 = getelementptr inbounds float addrspace(1)* %output, i64 %725
  %727 = extractelement <16 x i32> %711, i32 5
  %728 = sext i32 %727 to i64
  %729 = getelementptr inbounds float addrspace(1)* %output, i64 %728
  %730 = extractelement <16 x i32> %711, i32 6
  %731 = sext i32 %730 to i64
  %732 = getelementptr inbounds float addrspace(1)* %output, i64 %731
  %733 = extractelement <16 x i32> %711, i32 7
  %734 = sext i32 %733 to i64
  %735 = getelementptr inbounds float addrspace(1)* %output, i64 %734
  %736 = extractelement <16 x i32> %711, i32 8
  %737 = sext i32 %736 to i64
  %738 = getelementptr inbounds float addrspace(1)* %output, i64 %737
  %739 = extractelement <16 x i32> %711, i32 9
  %740 = sext i32 %739 to i64
  %741 = getelementptr inbounds float addrspace(1)* %output, i64 %740
  %742 = extractelement <16 x i32> %711, i32 10
  %743 = sext i32 %742 to i64
  %744 = getelementptr inbounds float addrspace(1)* %output, i64 %743
  %745 = extractelement <16 x i32> %711, i32 11
  %746 = sext i32 %745 to i64
  %747 = getelementptr inbounds float addrspace(1)* %output, i64 %746
  %748 = extractelement <16 x i32> %711, i32 12
  %749 = sext i32 %748 to i64
  %750 = getelementptr inbounds float addrspace(1)* %output, i64 %749
  %751 = extractelement <16 x i32> %711, i32 13
  %752 = sext i32 %751 to i64
  %753 = getelementptr inbounds float addrspace(1)* %output, i64 %752
  %754 = extractelement <16 x i32> %711, i32 14
  %755 = sext i32 %754 to i64
  %756 = getelementptr inbounds float addrspace(1)* %output, i64 %755
  %757 = extractelement <16 x i32> %711, i32 15
  %758 = sext i32 %757 to i64
  %759 = getelementptr inbounds float addrspace(1)* %output, i64 %758
  store float %extract16, float addrspace(1)* %714, align 4
  store float %extract17, float addrspace(1)* %717, align 4
  store float %extract18, float addrspace(1)* %720, align 4
  store float %extract19, float addrspace(1)* %723, align 4
  store float %extract20, float addrspace(1)* %726, align 4
  store float %extract21, float addrspace(1)* %729, align 4
  store float %extract22, float addrspace(1)* %732, align 4
  store float %extract23, float addrspace(1)* %735, align 4
  store float %extract24, float addrspace(1)* %738, align 4
  store float %extract25, float addrspace(1)* %741, align 4
  store float %extract26, float addrspace(1)* %744, align 4
  store float %extract27, float addrspace(1)* %747, align 4
  store float %extract28, float addrspace(1)* %750, align 4
  store float %extract29, float addrspace(1)* %753, align 4
  store float %extract30, float addrspace(1)* %756, align 4
  store float %extract31, float addrspace(1)* %759, align 4
  %760 = add nsw <16 x i32> %vectorPHI, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %761 = and <16 x i32> %760, %vector
  %762 = extractelement <16 x i32> %761, i32 0
  %763 = sext i32 %762 to i64
  %764 = getelementptr inbounds float addrspace(1)* %output, i64 %763
  %765 = extractelement <16 x i32> %761, i32 1
  %766 = sext i32 %765 to i64
  %767 = getelementptr inbounds float addrspace(1)* %output, i64 %766
  %768 = extractelement <16 x i32> %761, i32 2
  %769 = sext i32 %768 to i64
  %770 = getelementptr inbounds float addrspace(1)* %output, i64 %769
  %771 = extractelement <16 x i32> %761, i32 3
  %772 = sext i32 %771 to i64
  %773 = getelementptr inbounds float addrspace(1)* %output, i64 %772
  %774 = extractelement <16 x i32> %761, i32 4
  %775 = sext i32 %774 to i64
  %776 = getelementptr inbounds float addrspace(1)* %output, i64 %775
  %777 = extractelement <16 x i32> %761, i32 5
  %778 = sext i32 %777 to i64
  %779 = getelementptr inbounds float addrspace(1)* %output, i64 %778
  %780 = extractelement <16 x i32> %761, i32 6
  %781 = sext i32 %780 to i64
  %782 = getelementptr inbounds float addrspace(1)* %output, i64 %781
  %783 = extractelement <16 x i32> %761, i32 7
  %784 = sext i32 %783 to i64
  %785 = getelementptr inbounds float addrspace(1)* %output, i64 %784
  %786 = extractelement <16 x i32> %761, i32 8
  %787 = sext i32 %786 to i64
  %788 = getelementptr inbounds float addrspace(1)* %output, i64 %787
  %789 = extractelement <16 x i32> %761, i32 9
  %790 = sext i32 %789 to i64
  %791 = getelementptr inbounds float addrspace(1)* %output, i64 %790
  %792 = extractelement <16 x i32> %761, i32 10
  %793 = sext i32 %792 to i64
  %794 = getelementptr inbounds float addrspace(1)* %output, i64 %793
  %795 = extractelement <16 x i32> %761, i32 11
  %796 = sext i32 %795 to i64
  %797 = getelementptr inbounds float addrspace(1)* %output, i64 %796
  %798 = extractelement <16 x i32> %761, i32 12
  %799 = sext i32 %798 to i64
  %800 = getelementptr inbounds float addrspace(1)* %output, i64 %799
  %801 = extractelement <16 x i32> %761, i32 13
  %802 = sext i32 %801 to i64
  %803 = getelementptr inbounds float addrspace(1)* %output, i64 %802
  %804 = extractelement <16 x i32> %761, i32 14
  %805 = sext i32 %804 to i64
  %806 = getelementptr inbounds float addrspace(1)* %output, i64 %805
  %807 = extractelement <16 x i32> %761, i32 15
  %808 = sext i32 %807 to i64
  %809 = getelementptr inbounds float addrspace(1)* %output, i64 %808
  store float %extract16, float addrspace(1)* %764, align 4
  store float %extract17, float addrspace(1)* %767, align 4
  store float %extract18, float addrspace(1)* %770, align 4
  store float %extract19, float addrspace(1)* %773, align 4
  store float %extract20, float addrspace(1)* %776, align 4
  store float %extract21, float addrspace(1)* %779, align 4
  store float %extract22, float addrspace(1)* %782, align 4
  store float %extract23, float addrspace(1)* %785, align 4
  store float %extract24, float addrspace(1)* %788, align 4
  store float %extract25, float addrspace(1)* %791, align 4
  store float %extract26, float addrspace(1)* %794, align 4
  store float %extract27, float addrspace(1)* %797, align 4
  store float %extract28, float addrspace(1)* %800, align 4
  store float %extract29, float addrspace(1)* %803, align 4
  store float %extract30, float addrspace(1)* %806, align 4
  store float %extract31, float addrspace(1)* %809, align 4
  %810 = add nsw i32 %j.01, 1
  %811 = add nsw <16 x i32> %vectorPHI, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %812 = and <16 x i32> %811, %vector
  %exitcond = icmp eq i32 %810, 512
  br i1 %exitcond, label %._crit_edge, label %10

._crit_edge:                                      ; preds = %10
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB272

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB272:                                        ; preds = %._crit_edge
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
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
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

; <label>:23                                      ; preds = %23, %SyncBB.i
  %s.02.i = phi i32 [ %21, %SyncBB.i ], [ %89, %23 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %87, %23 ]
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
  br label %SyncBB.i

__writeGlobalMemoryUnit_separated_args.exit:      ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.writeGlobalMemoryUnit(i8* %pBuffer) {
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
  %temp.i = insertelement <16 x i32> undef, i32 %14, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %broadcast1.i = insertelement <16 x i64> undef, i64 %19, i32 0
  %broadcast2.i = shufflevector <16 x i64> %broadcast1.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %20 = add <16 x i64> %broadcast2.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %21 = trunc <16 x i64> %20 to <16 x i32>
  %22 = shl <16 x i32> %21, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %23 = sitofp <16 x i32> %21 to <16 x float>
  %extract16.i = extractelement <16 x float> %23, i32 0
  %extract17.i = extractelement <16 x float> %23, i32 1
  %extract18.i = extractelement <16 x float> %23, i32 2
  %extract19.i = extractelement <16 x float> %23, i32 3
  %extract20.i = extractelement <16 x float> %23, i32 4
  %extract21.i = extractelement <16 x float> %23, i32 5
  %extract22.i = extractelement <16 x float> %23, i32 6
  %extract23.i = extractelement <16 x float> %23, i32 7
  %extract24.i = extractelement <16 x float> %23, i32 8
  %extract25.i = extractelement <16 x float> %23, i32 9
  %extract26.i = extractelement <16 x float> %23, i32 10
  %extract27.i = extractelement <16 x float> %23, i32 11
  %extract28.i = extractelement <16 x float> %23, i32 12
  %extract29.i = extractelement <16 x float> %23, i32 13
  %extract30.i = extractelement <16 x float> %23, i32 14
  %extract31.i = extractelement <16 x float> %23, i32 15
  br label %24

; <label>:24                                      ; preds = %24, %SyncBB.i
  %vectorPHI.i = phi <16 x i32> [ %22, %SyncBB.i ], [ %826, %24 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %824, %24 ]
  %25 = and <16 x i32> %vectorPHI.i, %vector.i
  %26 = extractelement <16 x i32> %25, i32 0
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds float addrspace(1)* %1, i64 %27
  %29 = extractelement <16 x i32> %25, i32 1
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds float addrspace(1)* %1, i64 %30
  %32 = extractelement <16 x i32> %25, i32 2
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds float addrspace(1)* %1, i64 %33
  %35 = extractelement <16 x i32> %25, i32 3
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds float addrspace(1)* %1, i64 %36
  %38 = extractelement <16 x i32> %25, i32 4
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds float addrspace(1)* %1, i64 %39
  %41 = extractelement <16 x i32> %25, i32 5
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds float addrspace(1)* %1, i64 %42
  %44 = extractelement <16 x i32> %25, i32 6
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds float addrspace(1)* %1, i64 %45
  %47 = extractelement <16 x i32> %25, i32 7
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds float addrspace(1)* %1, i64 %48
  %50 = extractelement <16 x i32> %25, i32 8
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds float addrspace(1)* %1, i64 %51
  %53 = extractelement <16 x i32> %25, i32 9
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds float addrspace(1)* %1, i64 %54
  %56 = extractelement <16 x i32> %25, i32 10
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float addrspace(1)* %1, i64 %57
  %59 = extractelement <16 x i32> %25, i32 11
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds float addrspace(1)* %1, i64 %60
  %62 = extractelement <16 x i32> %25, i32 12
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds float addrspace(1)* %1, i64 %63
  %65 = extractelement <16 x i32> %25, i32 13
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %1, i64 %66
  %68 = extractelement <16 x i32> %25, i32 14
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds float addrspace(1)* %1, i64 %69
  %71 = extractelement <16 x i32> %25, i32 15
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds float addrspace(1)* %1, i64 %72
  store float %extract16.i, float addrspace(1)* %28, align 4
  store float %extract17.i, float addrspace(1)* %31, align 4
  store float %extract18.i, float addrspace(1)* %34, align 4
  store float %extract19.i, float addrspace(1)* %37, align 4
  store float %extract20.i, float addrspace(1)* %40, align 4
  store float %extract21.i, float addrspace(1)* %43, align 4
  store float %extract22.i, float addrspace(1)* %46, align 4
  store float %extract23.i, float addrspace(1)* %49, align 4
  store float %extract24.i, float addrspace(1)* %52, align 4
  store float %extract25.i, float addrspace(1)* %55, align 4
  store float %extract26.i, float addrspace(1)* %58, align 4
  store float %extract27.i, float addrspace(1)* %61, align 4
  store float %extract28.i, float addrspace(1)* %64, align 4
  store float %extract29.i, float addrspace(1)* %67, align 4
  store float %extract30.i, float addrspace(1)* %70, align 4
  store float %extract31.i, float addrspace(1)* %73, align 4
  %74 = add nsw <16 x i32> %vectorPHI.i, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %75 = and <16 x i32> %74, %vector.i
  %76 = extractelement <16 x i32> %75, i32 0
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %77
  %79 = extractelement <16 x i32> %75, i32 1
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %1, i64 %80
  %82 = extractelement <16 x i32> %75, i32 2
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds float addrspace(1)* %1, i64 %83
  %85 = extractelement <16 x i32> %75, i32 3
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds float addrspace(1)* %1, i64 %86
  %88 = extractelement <16 x i32> %75, i32 4
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds float addrspace(1)* %1, i64 %89
  %91 = extractelement <16 x i32> %75, i32 5
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds float addrspace(1)* %1, i64 %92
  %94 = extractelement <16 x i32> %75, i32 6
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds float addrspace(1)* %1, i64 %95
  %97 = extractelement <16 x i32> %75, i32 7
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds float addrspace(1)* %1, i64 %98
  %100 = extractelement <16 x i32> %75, i32 8
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %1, i64 %101
  %103 = extractelement <16 x i32> %75, i32 9
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds float addrspace(1)* %1, i64 %104
  %106 = extractelement <16 x i32> %75, i32 10
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds float addrspace(1)* %1, i64 %107
  %109 = extractelement <16 x i32> %75, i32 11
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds float addrspace(1)* %1, i64 %110
  %112 = extractelement <16 x i32> %75, i32 12
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds float addrspace(1)* %1, i64 %113
  %115 = extractelement <16 x i32> %75, i32 13
  %116 = sext i32 %115 to i64
  %117 = getelementptr inbounds float addrspace(1)* %1, i64 %116
  %118 = extractelement <16 x i32> %75, i32 14
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds float addrspace(1)* %1, i64 %119
  %121 = extractelement <16 x i32> %75, i32 15
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds float addrspace(1)* %1, i64 %122
  store float %extract16.i, float addrspace(1)* %78, align 4
  store float %extract17.i, float addrspace(1)* %81, align 4
  store float %extract18.i, float addrspace(1)* %84, align 4
  store float %extract19.i, float addrspace(1)* %87, align 4
  store float %extract20.i, float addrspace(1)* %90, align 4
  store float %extract21.i, float addrspace(1)* %93, align 4
  store float %extract22.i, float addrspace(1)* %96, align 4
  store float %extract23.i, float addrspace(1)* %99, align 4
  store float %extract24.i, float addrspace(1)* %102, align 4
  store float %extract25.i, float addrspace(1)* %105, align 4
  store float %extract26.i, float addrspace(1)* %108, align 4
  store float %extract27.i, float addrspace(1)* %111, align 4
  store float %extract28.i, float addrspace(1)* %114, align 4
  store float %extract29.i, float addrspace(1)* %117, align 4
  store float %extract30.i, float addrspace(1)* %120, align 4
  store float %extract31.i, float addrspace(1)* %123, align 4
  %124 = add nsw <16 x i32> %vectorPHI.i, <i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %125 = and <16 x i32> %124, %vector.i
  %126 = extractelement <16 x i32> %125, i32 0
  %127 = sext i32 %126 to i64
  %128 = getelementptr inbounds float addrspace(1)* %1, i64 %127
  %129 = extractelement <16 x i32> %125, i32 1
  %130 = sext i32 %129 to i64
  %131 = getelementptr inbounds float addrspace(1)* %1, i64 %130
  %132 = extractelement <16 x i32> %125, i32 2
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds float addrspace(1)* %1, i64 %133
  %135 = extractelement <16 x i32> %125, i32 3
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(1)* %1, i64 %136
  %138 = extractelement <16 x i32> %125, i32 4
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds float addrspace(1)* %1, i64 %139
  %141 = extractelement <16 x i32> %125, i32 5
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds float addrspace(1)* %1, i64 %142
  %144 = extractelement <16 x i32> %125, i32 6
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds float addrspace(1)* %1, i64 %145
  %147 = extractelement <16 x i32> %125, i32 7
  %148 = sext i32 %147 to i64
  %149 = getelementptr inbounds float addrspace(1)* %1, i64 %148
  %150 = extractelement <16 x i32> %125, i32 8
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds float addrspace(1)* %1, i64 %151
  %153 = extractelement <16 x i32> %125, i32 9
  %154 = sext i32 %153 to i64
  %155 = getelementptr inbounds float addrspace(1)* %1, i64 %154
  %156 = extractelement <16 x i32> %125, i32 10
  %157 = sext i32 %156 to i64
  %158 = getelementptr inbounds float addrspace(1)* %1, i64 %157
  %159 = extractelement <16 x i32> %125, i32 11
  %160 = sext i32 %159 to i64
  %161 = getelementptr inbounds float addrspace(1)* %1, i64 %160
  %162 = extractelement <16 x i32> %125, i32 12
  %163 = sext i32 %162 to i64
  %164 = getelementptr inbounds float addrspace(1)* %1, i64 %163
  %165 = extractelement <16 x i32> %125, i32 13
  %166 = sext i32 %165 to i64
  %167 = getelementptr inbounds float addrspace(1)* %1, i64 %166
  %168 = extractelement <16 x i32> %125, i32 14
  %169 = sext i32 %168 to i64
  %170 = getelementptr inbounds float addrspace(1)* %1, i64 %169
  %171 = extractelement <16 x i32> %125, i32 15
  %172 = sext i32 %171 to i64
  %173 = getelementptr inbounds float addrspace(1)* %1, i64 %172
  store float %extract16.i, float addrspace(1)* %128, align 4
  store float %extract17.i, float addrspace(1)* %131, align 4
  store float %extract18.i, float addrspace(1)* %134, align 4
  store float %extract19.i, float addrspace(1)* %137, align 4
  store float %extract20.i, float addrspace(1)* %140, align 4
  store float %extract21.i, float addrspace(1)* %143, align 4
  store float %extract22.i, float addrspace(1)* %146, align 4
  store float %extract23.i, float addrspace(1)* %149, align 4
  store float %extract24.i, float addrspace(1)* %152, align 4
  store float %extract25.i, float addrspace(1)* %155, align 4
  store float %extract26.i, float addrspace(1)* %158, align 4
  store float %extract27.i, float addrspace(1)* %161, align 4
  store float %extract28.i, float addrspace(1)* %164, align 4
  store float %extract29.i, float addrspace(1)* %167, align 4
  store float %extract30.i, float addrspace(1)* %170, align 4
  store float %extract31.i, float addrspace(1)* %173, align 4
  %174 = add nsw <16 x i32> %vectorPHI.i, <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
  %175 = and <16 x i32> %174, %vector.i
  %176 = extractelement <16 x i32> %175, i32 0
  %177 = sext i32 %176 to i64
  %178 = getelementptr inbounds float addrspace(1)* %1, i64 %177
  %179 = extractelement <16 x i32> %175, i32 1
  %180 = sext i32 %179 to i64
  %181 = getelementptr inbounds float addrspace(1)* %1, i64 %180
  %182 = extractelement <16 x i32> %175, i32 2
  %183 = sext i32 %182 to i64
  %184 = getelementptr inbounds float addrspace(1)* %1, i64 %183
  %185 = extractelement <16 x i32> %175, i32 3
  %186 = sext i32 %185 to i64
  %187 = getelementptr inbounds float addrspace(1)* %1, i64 %186
  %188 = extractelement <16 x i32> %175, i32 4
  %189 = sext i32 %188 to i64
  %190 = getelementptr inbounds float addrspace(1)* %1, i64 %189
  %191 = extractelement <16 x i32> %175, i32 5
  %192 = sext i32 %191 to i64
  %193 = getelementptr inbounds float addrspace(1)* %1, i64 %192
  %194 = extractelement <16 x i32> %175, i32 6
  %195 = sext i32 %194 to i64
  %196 = getelementptr inbounds float addrspace(1)* %1, i64 %195
  %197 = extractelement <16 x i32> %175, i32 7
  %198 = sext i32 %197 to i64
  %199 = getelementptr inbounds float addrspace(1)* %1, i64 %198
  %200 = extractelement <16 x i32> %175, i32 8
  %201 = sext i32 %200 to i64
  %202 = getelementptr inbounds float addrspace(1)* %1, i64 %201
  %203 = extractelement <16 x i32> %175, i32 9
  %204 = sext i32 %203 to i64
  %205 = getelementptr inbounds float addrspace(1)* %1, i64 %204
  %206 = extractelement <16 x i32> %175, i32 10
  %207 = sext i32 %206 to i64
  %208 = getelementptr inbounds float addrspace(1)* %1, i64 %207
  %209 = extractelement <16 x i32> %175, i32 11
  %210 = sext i32 %209 to i64
  %211 = getelementptr inbounds float addrspace(1)* %1, i64 %210
  %212 = extractelement <16 x i32> %175, i32 12
  %213 = sext i32 %212 to i64
  %214 = getelementptr inbounds float addrspace(1)* %1, i64 %213
  %215 = extractelement <16 x i32> %175, i32 13
  %216 = sext i32 %215 to i64
  %217 = getelementptr inbounds float addrspace(1)* %1, i64 %216
  %218 = extractelement <16 x i32> %175, i32 14
  %219 = sext i32 %218 to i64
  %220 = getelementptr inbounds float addrspace(1)* %1, i64 %219
  %221 = extractelement <16 x i32> %175, i32 15
  %222 = sext i32 %221 to i64
  %223 = getelementptr inbounds float addrspace(1)* %1, i64 %222
  store float %extract16.i, float addrspace(1)* %178, align 4
  store float %extract17.i, float addrspace(1)* %181, align 4
  store float %extract18.i, float addrspace(1)* %184, align 4
  store float %extract19.i, float addrspace(1)* %187, align 4
  store float %extract20.i, float addrspace(1)* %190, align 4
  store float %extract21.i, float addrspace(1)* %193, align 4
  store float %extract22.i, float addrspace(1)* %196, align 4
  store float %extract23.i, float addrspace(1)* %199, align 4
  store float %extract24.i, float addrspace(1)* %202, align 4
  store float %extract25.i, float addrspace(1)* %205, align 4
  store float %extract26.i, float addrspace(1)* %208, align 4
  store float %extract27.i, float addrspace(1)* %211, align 4
  store float %extract28.i, float addrspace(1)* %214, align 4
  store float %extract29.i, float addrspace(1)* %217, align 4
  store float %extract30.i, float addrspace(1)* %220, align 4
  store float %extract31.i, float addrspace(1)* %223, align 4
  %224 = add nsw <16 x i32> %vectorPHI.i, <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
  %225 = and <16 x i32> %224, %vector.i
  %226 = extractelement <16 x i32> %225, i32 0
  %227 = sext i32 %226 to i64
  %228 = getelementptr inbounds float addrspace(1)* %1, i64 %227
  %229 = extractelement <16 x i32> %225, i32 1
  %230 = sext i32 %229 to i64
  %231 = getelementptr inbounds float addrspace(1)* %1, i64 %230
  %232 = extractelement <16 x i32> %225, i32 2
  %233 = sext i32 %232 to i64
  %234 = getelementptr inbounds float addrspace(1)* %1, i64 %233
  %235 = extractelement <16 x i32> %225, i32 3
  %236 = sext i32 %235 to i64
  %237 = getelementptr inbounds float addrspace(1)* %1, i64 %236
  %238 = extractelement <16 x i32> %225, i32 4
  %239 = sext i32 %238 to i64
  %240 = getelementptr inbounds float addrspace(1)* %1, i64 %239
  %241 = extractelement <16 x i32> %225, i32 5
  %242 = sext i32 %241 to i64
  %243 = getelementptr inbounds float addrspace(1)* %1, i64 %242
  %244 = extractelement <16 x i32> %225, i32 6
  %245 = sext i32 %244 to i64
  %246 = getelementptr inbounds float addrspace(1)* %1, i64 %245
  %247 = extractelement <16 x i32> %225, i32 7
  %248 = sext i32 %247 to i64
  %249 = getelementptr inbounds float addrspace(1)* %1, i64 %248
  %250 = extractelement <16 x i32> %225, i32 8
  %251 = sext i32 %250 to i64
  %252 = getelementptr inbounds float addrspace(1)* %1, i64 %251
  %253 = extractelement <16 x i32> %225, i32 9
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds float addrspace(1)* %1, i64 %254
  %256 = extractelement <16 x i32> %225, i32 10
  %257 = sext i32 %256 to i64
  %258 = getelementptr inbounds float addrspace(1)* %1, i64 %257
  %259 = extractelement <16 x i32> %225, i32 11
  %260 = sext i32 %259 to i64
  %261 = getelementptr inbounds float addrspace(1)* %1, i64 %260
  %262 = extractelement <16 x i32> %225, i32 12
  %263 = sext i32 %262 to i64
  %264 = getelementptr inbounds float addrspace(1)* %1, i64 %263
  %265 = extractelement <16 x i32> %225, i32 13
  %266 = sext i32 %265 to i64
  %267 = getelementptr inbounds float addrspace(1)* %1, i64 %266
  %268 = extractelement <16 x i32> %225, i32 14
  %269 = sext i32 %268 to i64
  %270 = getelementptr inbounds float addrspace(1)* %1, i64 %269
  %271 = extractelement <16 x i32> %225, i32 15
  %272 = sext i32 %271 to i64
  %273 = getelementptr inbounds float addrspace(1)* %1, i64 %272
  store float %extract16.i, float addrspace(1)* %228, align 4
  store float %extract17.i, float addrspace(1)* %231, align 4
  store float %extract18.i, float addrspace(1)* %234, align 4
  store float %extract19.i, float addrspace(1)* %237, align 4
  store float %extract20.i, float addrspace(1)* %240, align 4
  store float %extract21.i, float addrspace(1)* %243, align 4
  store float %extract22.i, float addrspace(1)* %246, align 4
  store float %extract23.i, float addrspace(1)* %249, align 4
  store float %extract24.i, float addrspace(1)* %252, align 4
  store float %extract25.i, float addrspace(1)* %255, align 4
  store float %extract26.i, float addrspace(1)* %258, align 4
  store float %extract27.i, float addrspace(1)* %261, align 4
  store float %extract28.i, float addrspace(1)* %264, align 4
  store float %extract29.i, float addrspace(1)* %267, align 4
  store float %extract30.i, float addrspace(1)* %270, align 4
  store float %extract31.i, float addrspace(1)* %273, align 4
  %274 = add nsw <16 x i32> %vectorPHI.i, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  %275 = and <16 x i32> %274, %vector.i
  %276 = extractelement <16 x i32> %275, i32 0
  %277 = sext i32 %276 to i64
  %278 = getelementptr inbounds float addrspace(1)* %1, i64 %277
  %279 = extractelement <16 x i32> %275, i32 1
  %280 = sext i32 %279 to i64
  %281 = getelementptr inbounds float addrspace(1)* %1, i64 %280
  %282 = extractelement <16 x i32> %275, i32 2
  %283 = sext i32 %282 to i64
  %284 = getelementptr inbounds float addrspace(1)* %1, i64 %283
  %285 = extractelement <16 x i32> %275, i32 3
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds float addrspace(1)* %1, i64 %286
  %288 = extractelement <16 x i32> %275, i32 4
  %289 = sext i32 %288 to i64
  %290 = getelementptr inbounds float addrspace(1)* %1, i64 %289
  %291 = extractelement <16 x i32> %275, i32 5
  %292 = sext i32 %291 to i64
  %293 = getelementptr inbounds float addrspace(1)* %1, i64 %292
  %294 = extractelement <16 x i32> %275, i32 6
  %295 = sext i32 %294 to i64
  %296 = getelementptr inbounds float addrspace(1)* %1, i64 %295
  %297 = extractelement <16 x i32> %275, i32 7
  %298 = sext i32 %297 to i64
  %299 = getelementptr inbounds float addrspace(1)* %1, i64 %298
  %300 = extractelement <16 x i32> %275, i32 8
  %301 = sext i32 %300 to i64
  %302 = getelementptr inbounds float addrspace(1)* %1, i64 %301
  %303 = extractelement <16 x i32> %275, i32 9
  %304 = sext i32 %303 to i64
  %305 = getelementptr inbounds float addrspace(1)* %1, i64 %304
  %306 = extractelement <16 x i32> %275, i32 10
  %307 = sext i32 %306 to i64
  %308 = getelementptr inbounds float addrspace(1)* %1, i64 %307
  %309 = extractelement <16 x i32> %275, i32 11
  %310 = sext i32 %309 to i64
  %311 = getelementptr inbounds float addrspace(1)* %1, i64 %310
  %312 = extractelement <16 x i32> %275, i32 12
  %313 = sext i32 %312 to i64
  %314 = getelementptr inbounds float addrspace(1)* %1, i64 %313
  %315 = extractelement <16 x i32> %275, i32 13
  %316 = sext i32 %315 to i64
  %317 = getelementptr inbounds float addrspace(1)* %1, i64 %316
  %318 = extractelement <16 x i32> %275, i32 14
  %319 = sext i32 %318 to i64
  %320 = getelementptr inbounds float addrspace(1)* %1, i64 %319
  %321 = extractelement <16 x i32> %275, i32 15
  %322 = sext i32 %321 to i64
  %323 = getelementptr inbounds float addrspace(1)* %1, i64 %322
  store float %extract16.i, float addrspace(1)* %278, align 4
  store float %extract17.i, float addrspace(1)* %281, align 4
  store float %extract18.i, float addrspace(1)* %284, align 4
  store float %extract19.i, float addrspace(1)* %287, align 4
  store float %extract20.i, float addrspace(1)* %290, align 4
  store float %extract21.i, float addrspace(1)* %293, align 4
  store float %extract22.i, float addrspace(1)* %296, align 4
  store float %extract23.i, float addrspace(1)* %299, align 4
  store float %extract24.i, float addrspace(1)* %302, align 4
  store float %extract25.i, float addrspace(1)* %305, align 4
  store float %extract26.i, float addrspace(1)* %308, align 4
  store float %extract27.i, float addrspace(1)* %311, align 4
  store float %extract28.i, float addrspace(1)* %314, align 4
  store float %extract29.i, float addrspace(1)* %317, align 4
  store float %extract30.i, float addrspace(1)* %320, align 4
  store float %extract31.i, float addrspace(1)* %323, align 4
  %324 = add nsw <16 x i32> %vectorPHI.i, <i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6, i32 6>
  %325 = and <16 x i32> %324, %vector.i
  %326 = extractelement <16 x i32> %325, i32 0
  %327 = sext i32 %326 to i64
  %328 = getelementptr inbounds float addrspace(1)* %1, i64 %327
  %329 = extractelement <16 x i32> %325, i32 1
  %330 = sext i32 %329 to i64
  %331 = getelementptr inbounds float addrspace(1)* %1, i64 %330
  %332 = extractelement <16 x i32> %325, i32 2
  %333 = sext i32 %332 to i64
  %334 = getelementptr inbounds float addrspace(1)* %1, i64 %333
  %335 = extractelement <16 x i32> %325, i32 3
  %336 = sext i32 %335 to i64
  %337 = getelementptr inbounds float addrspace(1)* %1, i64 %336
  %338 = extractelement <16 x i32> %325, i32 4
  %339 = sext i32 %338 to i64
  %340 = getelementptr inbounds float addrspace(1)* %1, i64 %339
  %341 = extractelement <16 x i32> %325, i32 5
  %342 = sext i32 %341 to i64
  %343 = getelementptr inbounds float addrspace(1)* %1, i64 %342
  %344 = extractelement <16 x i32> %325, i32 6
  %345 = sext i32 %344 to i64
  %346 = getelementptr inbounds float addrspace(1)* %1, i64 %345
  %347 = extractelement <16 x i32> %325, i32 7
  %348 = sext i32 %347 to i64
  %349 = getelementptr inbounds float addrspace(1)* %1, i64 %348
  %350 = extractelement <16 x i32> %325, i32 8
  %351 = sext i32 %350 to i64
  %352 = getelementptr inbounds float addrspace(1)* %1, i64 %351
  %353 = extractelement <16 x i32> %325, i32 9
  %354 = sext i32 %353 to i64
  %355 = getelementptr inbounds float addrspace(1)* %1, i64 %354
  %356 = extractelement <16 x i32> %325, i32 10
  %357 = sext i32 %356 to i64
  %358 = getelementptr inbounds float addrspace(1)* %1, i64 %357
  %359 = extractelement <16 x i32> %325, i32 11
  %360 = sext i32 %359 to i64
  %361 = getelementptr inbounds float addrspace(1)* %1, i64 %360
  %362 = extractelement <16 x i32> %325, i32 12
  %363 = sext i32 %362 to i64
  %364 = getelementptr inbounds float addrspace(1)* %1, i64 %363
  %365 = extractelement <16 x i32> %325, i32 13
  %366 = sext i32 %365 to i64
  %367 = getelementptr inbounds float addrspace(1)* %1, i64 %366
  %368 = extractelement <16 x i32> %325, i32 14
  %369 = sext i32 %368 to i64
  %370 = getelementptr inbounds float addrspace(1)* %1, i64 %369
  %371 = extractelement <16 x i32> %325, i32 15
  %372 = sext i32 %371 to i64
  %373 = getelementptr inbounds float addrspace(1)* %1, i64 %372
  store float %extract16.i, float addrspace(1)* %328, align 4
  store float %extract17.i, float addrspace(1)* %331, align 4
  store float %extract18.i, float addrspace(1)* %334, align 4
  store float %extract19.i, float addrspace(1)* %337, align 4
  store float %extract20.i, float addrspace(1)* %340, align 4
  store float %extract21.i, float addrspace(1)* %343, align 4
  store float %extract22.i, float addrspace(1)* %346, align 4
  store float %extract23.i, float addrspace(1)* %349, align 4
  store float %extract24.i, float addrspace(1)* %352, align 4
  store float %extract25.i, float addrspace(1)* %355, align 4
  store float %extract26.i, float addrspace(1)* %358, align 4
  store float %extract27.i, float addrspace(1)* %361, align 4
  store float %extract28.i, float addrspace(1)* %364, align 4
  store float %extract29.i, float addrspace(1)* %367, align 4
  store float %extract30.i, float addrspace(1)* %370, align 4
  store float %extract31.i, float addrspace(1)* %373, align 4
  %374 = add nsw <16 x i32> %vectorPHI.i, <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
  %375 = and <16 x i32> %374, %vector.i
  %376 = extractelement <16 x i32> %375, i32 0
  %377 = sext i32 %376 to i64
  %378 = getelementptr inbounds float addrspace(1)* %1, i64 %377
  %379 = extractelement <16 x i32> %375, i32 1
  %380 = sext i32 %379 to i64
  %381 = getelementptr inbounds float addrspace(1)* %1, i64 %380
  %382 = extractelement <16 x i32> %375, i32 2
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds float addrspace(1)* %1, i64 %383
  %385 = extractelement <16 x i32> %375, i32 3
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds float addrspace(1)* %1, i64 %386
  %388 = extractelement <16 x i32> %375, i32 4
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds float addrspace(1)* %1, i64 %389
  %391 = extractelement <16 x i32> %375, i32 5
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds float addrspace(1)* %1, i64 %392
  %394 = extractelement <16 x i32> %375, i32 6
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds float addrspace(1)* %1, i64 %395
  %397 = extractelement <16 x i32> %375, i32 7
  %398 = sext i32 %397 to i64
  %399 = getelementptr inbounds float addrspace(1)* %1, i64 %398
  %400 = extractelement <16 x i32> %375, i32 8
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds float addrspace(1)* %1, i64 %401
  %403 = extractelement <16 x i32> %375, i32 9
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds float addrspace(1)* %1, i64 %404
  %406 = extractelement <16 x i32> %375, i32 10
  %407 = sext i32 %406 to i64
  %408 = getelementptr inbounds float addrspace(1)* %1, i64 %407
  %409 = extractelement <16 x i32> %375, i32 11
  %410 = sext i32 %409 to i64
  %411 = getelementptr inbounds float addrspace(1)* %1, i64 %410
  %412 = extractelement <16 x i32> %375, i32 12
  %413 = sext i32 %412 to i64
  %414 = getelementptr inbounds float addrspace(1)* %1, i64 %413
  %415 = extractelement <16 x i32> %375, i32 13
  %416 = sext i32 %415 to i64
  %417 = getelementptr inbounds float addrspace(1)* %1, i64 %416
  %418 = extractelement <16 x i32> %375, i32 14
  %419 = sext i32 %418 to i64
  %420 = getelementptr inbounds float addrspace(1)* %1, i64 %419
  %421 = extractelement <16 x i32> %375, i32 15
  %422 = sext i32 %421 to i64
  %423 = getelementptr inbounds float addrspace(1)* %1, i64 %422
  store float %extract16.i, float addrspace(1)* %378, align 4
  store float %extract17.i, float addrspace(1)* %381, align 4
  store float %extract18.i, float addrspace(1)* %384, align 4
  store float %extract19.i, float addrspace(1)* %387, align 4
  store float %extract20.i, float addrspace(1)* %390, align 4
  store float %extract21.i, float addrspace(1)* %393, align 4
  store float %extract22.i, float addrspace(1)* %396, align 4
  store float %extract23.i, float addrspace(1)* %399, align 4
  store float %extract24.i, float addrspace(1)* %402, align 4
  store float %extract25.i, float addrspace(1)* %405, align 4
  store float %extract26.i, float addrspace(1)* %408, align 4
  store float %extract27.i, float addrspace(1)* %411, align 4
  store float %extract28.i, float addrspace(1)* %414, align 4
  store float %extract29.i, float addrspace(1)* %417, align 4
  store float %extract30.i, float addrspace(1)* %420, align 4
  store float %extract31.i, float addrspace(1)* %423, align 4
  %424 = add nsw <16 x i32> %vectorPHI.i, <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
  %425 = and <16 x i32> %424, %vector.i
  %426 = extractelement <16 x i32> %425, i32 0
  %427 = sext i32 %426 to i64
  %428 = getelementptr inbounds float addrspace(1)* %1, i64 %427
  %429 = extractelement <16 x i32> %425, i32 1
  %430 = sext i32 %429 to i64
  %431 = getelementptr inbounds float addrspace(1)* %1, i64 %430
  %432 = extractelement <16 x i32> %425, i32 2
  %433 = sext i32 %432 to i64
  %434 = getelementptr inbounds float addrspace(1)* %1, i64 %433
  %435 = extractelement <16 x i32> %425, i32 3
  %436 = sext i32 %435 to i64
  %437 = getelementptr inbounds float addrspace(1)* %1, i64 %436
  %438 = extractelement <16 x i32> %425, i32 4
  %439 = sext i32 %438 to i64
  %440 = getelementptr inbounds float addrspace(1)* %1, i64 %439
  %441 = extractelement <16 x i32> %425, i32 5
  %442 = sext i32 %441 to i64
  %443 = getelementptr inbounds float addrspace(1)* %1, i64 %442
  %444 = extractelement <16 x i32> %425, i32 6
  %445 = sext i32 %444 to i64
  %446 = getelementptr inbounds float addrspace(1)* %1, i64 %445
  %447 = extractelement <16 x i32> %425, i32 7
  %448 = sext i32 %447 to i64
  %449 = getelementptr inbounds float addrspace(1)* %1, i64 %448
  %450 = extractelement <16 x i32> %425, i32 8
  %451 = sext i32 %450 to i64
  %452 = getelementptr inbounds float addrspace(1)* %1, i64 %451
  %453 = extractelement <16 x i32> %425, i32 9
  %454 = sext i32 %453 to i64
  %455 = getelementptr inbounds float addrspace(1)* %1, i64 %454
  %456 = extractelement <16 x i32> %425, i32 10
  %457 = sext i32 %456 to i64
  %458 = getelementptr inbounds float addrspace(1)* %1, i64 %457
  %459 = extractelement <16 x i32> %425, i32 11
  %460 = sext i32 %459 to i64
  %461 = getelementptr inbounds float addrspace(1)* %1, i64 %460
  %462 = extractelement <16 x i32> %425, i32 12
  %463 = sext i32 %462 to i64
  %464 = getelementptr inbounds float addrspace(1)* %1, i64 %463
  %465 = extractelement <16 x i32> %425, i32 13
  %466 = sext i32 %465 to i64
  %467 = getelementptr inbounds float addrspace(1)* %1, i64 %466
  %468 = extractelement <16 x i32> %425, i32 14
  %469 = sext i32 %468 to i64
  %470 = getelementptr inbounds float addrspace(1)* %1, i64 %469
  %471 = extractelement <16 x i32> %425, i32 15
  %472 = sext i32 %471 to i64
  %473 = getelementptr inbounds float addrspace(1)* %1, i64 %472
  store float %extract16.i, float addrspace(1)* %428, align 4
  store float %extract17.i, float addrspace(1)* %431, align 4
  store float %extract18.i, float addrspace(1)* %434, align 4
  store float %extract19.i, float addrspace(1)* %437, align 4
  store float %extract20.i, float addrspace(1)* %440, align 4
  store float %extract21.i, float addrspace(1)* %443, align 4
  store float %extract22.i, float addrspace(1)* %446, align 4
  store float %extract23.i, float addrspace(1)* %449, align 4
  store float %extract24.i, float addrspace(1)* %452, align 4
  store float %extract25.i, float addrspace(1)* %455, align 4
  store float %extract26.i, float addrspace(1)* %458, align 4
  store float %extract27.i, float addrspace(1)* %461, align 4
  store float %extract28.i, float addrspace(1)* %464, align 4
  store float %extract29.i, float addrspace(1)* %467, align 4
  store float %extract30.i, float addrspace(1)* %470, align 4
  store float %extract31.i, float addrspace(1)* %473, align 4
  %474 = add nsw <16 x i32> %vectorPHI.i, <i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9, i32 9>
  %475 = and <16 x i32> %474, %vector.i
  %476 = extractelement <16 x i32> %475, i32 0
  %477 = sext i32 %476 to i64
  %478 = getelementptr inbounds float addrspace(1)* %1, i64 %477
  %479 = extractelement <16 x i32> %475, i32 1
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds float addrspace(1)* %1, i64 %480
  %482 = extractelement <16 x i32> %475, i32 2
  %483 = sext i32 %482 to i64
  %484 = getelementptr inbounds float addrspace(1)* %1, i64 %483
  %485 = extractelement <16 x i32> %475, i32 3
  %486 = sext i32 %485 to i64
  %487 = getelementptr inbounds float addrspace(1)* %1, i64 %486
  %488 = extractelement <16 x i32> %475, i32 4
  %489 = sext i32 %488 to i64
  %490 = getelementptr inbounds float addrspace(1)* %1, i64 %489
  %491 = extractelement <16 x i32> %475, i32 5
  %492 = sext i32 %491 to i64
  %493 = getelementptr inbounds float addrspace(1)* %1, i64 %492
  %494 = extractelement <16 x i32> %475, i32 6
  %495 = sext i32 %494 to i64
  %496 = getelementptr inbounds float addrspace(1)* %1, i64 %495
  %497 = extractelement <16 x i32> %475, i32 7
  %498 = sext i32 %497 to i64
  %499 = getelementptr inbounds float addrspace(1)* %1, i64 %498
  %500 = extractelement <16 x i32> %475, i32 8
  %501 = sext i32 %500 to i64
  %502 = getelementptr inbounds float addrspace(1)* %1, i64 %501
  %503 = extractelement <16 x i32> %475, i32 9
  %504 = sext i32 %503 to i64
  %505 = getelementptr inbounds float addrspace(1)* %1, i64 %504
  %506 = extractelement <16 x i32> %475, i32 10
  %507 = sext i32 %506 to i64
  %508 = getelementptr inbounds float addrspace(1)* %1, i64 %507
  %509 = extractelement <16 x i32> %475, i32 11
  %510 = sext i32 %509 to i64
  %511 = getelementptr inbounds float addrspace(1)* %1, i64 %510
  %512 = extractelement <16 x i32> %475, i32 12
  %513 = sext i32 %512 to i64
  %514 = getelementptr inbounds float addrspace(1)* %1, i64 %513
  %515 = extractelement <16 x i32> %475, i32 13
  %516 = sext i32 %515 to i64
  %517 = getelementptr inbounds float addrspace(1)* %1, i64 %516
  %518 = extractelement <16 x i32> %475, i32 14
  %519 = sext i32 %518 to i64
  %520 = getelementptr inbounds float addrspace(1)* %1, i64 %519
  %521 = extractelement <16 x i32> %475, i32 15
  %522 = sext i32 %521 to i64
  %523 = getelementptr inbounds float addrspace(1)* %1, i64 %522
  store float %extract16.i, float addrspace(1)* %478, align 4
  store float %extract17.i, float addrspace(1)* %481, align 4
  store float %extract18.i, float addrspace(1)* %484, align 4
  store float %extract19.i, float addrspace(1)* %487, align 4
  store float %extract20.i, float addrspace(1)* %490, align 4
  store float %extract21.i, float addrspace(1)* %493, align 4
  store float %extract22.i, float addrspace(1)* %496, align 4
  store float %extract23.i, float addrspace(1)* %499, align 4
  store float %extract24.i, float addrspace(1)* %502, align 4
  store float %extract25.i, float addrspace(1)* %505, align 4
  store float %extract26.i, float addrspace(1)* %508, align 4
  store float %extract27.i, float addrspace(1)* %511, align 4
  store float %extract28.i, float addrspace(1)* %514, align 4
  store float %extract29.i, float addrspace(1)* %517, align 4
  store float %extract30.i, float addrspace(1)* %520, align 4
  store float %extract31.i, float addrspace(1)* %523, align 4
  %524 = add nsw <16 x i32> %vectorPHI.i, <i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10, i32 10>
  %525 = and <16 x i32> %524, %vector.i
  %526 = extractelement <16 x i32> %525, i32 0
  %527 = sext i32 %526 to i64
  %528 = getelementptr inbounds float addrspace(1)* %1, i64 %527
  %529 = extractelement <16 x i32> %525, i32 1
  %530 = sext i32 %529 to i64
  %531 = getelementptr inbounds float addrspace(1)* %1, i64 %530
  %532 = extractelement <16 x i32> %525, i32 2
  %533 = sext i32 %532 to i64
  %534 = getelementptr inbounds float addrspace(1)* %1, i64 %533
  %535 = extractelement <16 x i32> %525, i32 3
  %536 = sext i32 %535 to i64
  %537 = getelementptr inbounds float addrspace(1)* %1, i64 %536
  %538 = extractelement <16 x i32> %525, i32 4
  %539 = sext i32 %538 to i64
  %540 = getelementptr inbounds float addrspace(1)* %1, i64 %539
  %541 = extractelement <16 x i32> %525, i32 5
  %542 = sext i32 %541 to i64
  %543 = getelementptr inbounds float addrspace(1)* %1, i64 %542
  %544 = extractelement <16 x i32> %525, i32 6
  %545 = sext i32 %544 to i64
  %546 = getelementptr inbounds float addrspace(1)* %1, i64 %545
  %547 = extractelement <16 x i32> %525, i32 7
  %548 = sext i32 %547 to i64
  %549 = getelementptr inbounds float addrspace(1)* %1, i64 %548
  %550 = extractelement <16 x i32> %525, i32 8
  %551 = sext i32 %550 to i64
  %552 = getelementptr inbounds float addrspace(1)* %1, i64 %551
  %553 = extractelement <16 x i32> %525, i32 9
  %554 = sext i32 %553 to i64
  %555 = getelementptr inbounds float addrspace(1)* %1, i64 %554
  %556 = extractelement <16 x i32> %525, i32 10
  %557 = sext i32 %556 to i64
  %558 = getelementptr inbounds float addrspace(1)* %1, i64 %557
  %559 = extractelement <16 x i32> %525, i32 11
  %560 = sext i32 %559 to i64
  %561 = getelementptr inbounds float addrspace(1)* %1, i64 %560
  %562 = extractelement <16 x i32> %525, i32 12
  %563 = sext i32 %562 to i64
  %564 = getelementptr inbounds float addrspace(1)* %1, i64 %563
  %565 = extractelement <16 x i32> %525, i32 13
  %566 = sext i32 %565 to i64
  %567 = getelementptr inbounds float addrspace(1)* %1, i64 %566
  %568 = extractelement <16 x i32> %525, i32 14
  %569 = sext i32 %568 to i64
  %570 = getelementptr inbounds float addrspace(1)* %1, i64 %569
  %571 = extractelement <16 x i32> %525, i32 15
  %572 = sext i32 %571 to i64
  %573 = getelementptr inbounds float addrspace(1)* %1, i64 %572
  store float %extract16.i, float addrspace(1)* %528, align 4
  store float %extract17.i, float addrspace(1)* %531, align 4
  store float %extract18.i, float addrspace(1)* %534, align 4
  store float %extract19.i, float addrspace(1)* %537, align 4
  store float %extract20.i, float addrspace(1)* %540, align 4
  store float %extract21.i, float addrspace(1)* %543, align 4
  store float %extract22.i, float addrspace(1)* %546, align 4
  store float %extract23.i, float addrspace(1)* %549, align 4
  store float %extract24.i, float addrspace(1)* %552, align 4
  store float %extract25.i, float addrspace(1)* %555, align 4
  store float %extract26.i, float addrspace(1)* %558, align 4
  store float %extract27.i, float addrspace(1)* %561, align 4
  store float %extract28.i, float addrspace(1)* %564, align 4
  store float %extract29.i, float addrspace(1)* %567, align 4
  store float %extract30.i, float addrspace(1)* %570, align 4
  store float %extract31.i, float addrspace(1)* %573, align 4
  %574 = add nsw <16 x i32> %vectorPHI.i, <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
  %575 = and <16 x i32> %574, %vector.i
  %576 = extractelement <16 x i32> %575, i32 0
  %577 = sext i32 %576 to i64
  %578 = getelementptr inbounds float addrspace(1)* %1, i64 %577
  %579 = extractelement <16 x i32> %575, i32 1
  %580 = sext i32 %579 to i64
  %581 = getelementptr inbounds float addrspace(1)* %1, i64 %580
  %582 = extractelement <16 x i32> %575, i32 2
  %583 = sext i32 %582 to i64
  %584 = getelementptr inbounds float addrspace(1)* %1, i64 %583
  %585 = extractelement <16 x i32> %575, i32 3
  %586 = sext i32 %585 to i64
  %587 = getelementptr inbounds float addrspace(1)* %1, i64 %586
  %588 = extractelement <16 x i32> %575, i32 4
  %589 = sext i32 %588 to i64
  %590 = getelementptr inbounds float addrspace(1)* %1, i64 %589
  %591 = extractelement <16 x i32> %575, i32 5
  %592 = sext i32 %591 to i64
  %593 = getelementptr inbounds float addrspace(1)* %1, i64 %592
  %594 = extractelement <16 x i32> %575, i32 6
  %595 = sext i32 %594 to i64
  %596 = getelementptr inbounds float addrspace(1)* %1, i64 %595
  %597 = extractelement <16 x i32> %575, i32 7
  %598 = sext i32 %597 to i64
  %599 = getelementptr inbounds float addrspace(1)* %1, i64 %598
  %600 = extractelement <16 x i32> %575, i32 8
  %601 = sext i32 %600 to i64
  %602 = getelementptr inbounds float addrspace(1)* %1, i64 %601
  %603 = extractelement <16 x i32> %575, i32 9
  %604 = sext i32 %603 to i64
  %605 = getelementptr inbounds float addrspace(1)* %1, i64 %604
  %606 = extractelement <16 x i32> %575, i32 10
  %607 = sext i32 %606 to i64
  %608 = getelementptr inbounds float addrspace(1)* %1, i64 %607
  %609 = extractelement <16 x i32> %575, i32 11
  %610 = sext i32 %609 to i64
  %611 = getelementptr inbounds float addrspace(1)* %1, i64 %610
  %612 = extractelement <16 x i32> %575, i32 12
  %613 = sext i32 %612 to i64
  %614 = getelementptr inbounds float addrspace(1)* %1, i64 %613
  %615 = extractelement <16 x i32> %575, i32 13
  %616 = sext i32 %615 to i64
  %617 = getelementptr inbounds float addrspace(1)* %1, i64 %616
  %618 = extractelement <16 x i32> %575, i32 14
  %619 = sext i32 %618 to i64
  %620 = getelementptr inbounds float addrspace(1)* %1, i64 %619
  %621 = extractelement <16 x i32> %575, i32 15
  %622 = sext i32 %621 to i64
  %623 = getelementptr inbounds float addrspace(1)* %1, i64 %622
  store float %extract16.i, float addrspace(1)* %578, align 4
  store float %extract17.i, float addrspace(1)* %581, align 4
  store float %extract18.i, float addrspace(1)* %584, align 4
  store float %extract19.i, float addrspace(1)* %587, align 4
  store float %extract20.i, float addrspace(1)* %590, align 4
  store float %extract21.i, float addrspace(1)* %593, align 4
  store float %extract22.i, float addrspace(1)* %596, align 4
  store float %extract23.i, float addrspace(1)* %599, align 4
  store float %extract24.i, float addrspace(1)* %602, align 4
  store float %extract25.i, float addrspace(1)* %605, align 4
  store float %extract26.i, float addrspace(1)* %608, align 4
  store float %extract27.i, float addrspace(1)* %611, align 4
  store float %extract28.i, float addrspace(1)* %614, align 4
  store float %extract29.i, float addrspace(1)* %617, align 4
  store float %extract30.i, float addrspace(1)* %620, align 4
  store float %extract31.i, float addrspace(1)* %623, align 4
  %624 = add nsw <16 x i32> %vectorPHI.i, <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
  %625 = and <16 x i32> %624, %vector.i
  %626 = extractelement <16 x i32> %625, i32 0
  %627 = sext i32 %626 to i64
  %628 = getelementptr inbounds float addrspace(1)* %1, i64 %627
  %629 = extractelement <16 x i32> %625, i32 1
  %630 = sext i32 %629 to i64
  %631 = getelementptr inbounds float addrspace(1)* %1, i64 %630
  %632 = extractelement <16 x i32> %625, i32 2
  %633 = sext i32 %632 to i64
  %634 = getelementptr inbounds float addrspace(1)* %1, i64 %633
  %635 = extractelement <16 x i32> %625, i32 3
  %636 = sext i32 %635 to i64
  %637 = getelementptr inbounds float addrspace(1)* %1, i64 %636
  %638 = extractelement <16 x i32> %625, i32 4
  %639 = sext i32 %638 to i64
  %640 = getelementptr inbounds float addrspace(1)* %1, i64 %639
  %641 = extractelement <16 x i32> %625, i32 5
  %642 = sext i32 %641 to i64
  %643 = getelementptr inbounds float addrspace(1)* %1, i64 %642
  %644 = extractelement <16 x i32> %625, i32 6
  %645 = sext i32 %644 to i64
  %646 = getelementptr inbounds float addrspace(1)* %1, i64 %645
  %647 = extractelement <16 x i32> %625, i32 7
  %648 = sext i32 %647 to i64
  %649 = getelementptr inbounds float addrspace(1)* %1, i64 %648
  %650 = extractelement <16 x i32> %625, i32 8
  %651 = sext i32 %650 to i64
  %652 = getelementptr inbounds float addrspace(1)* %1, i64 %651
  %653 = extractelement <16 x i32> %625, i32 9
  %654 = sext i32 %653 to i64
  %655 = getelementptr inbounds float addrspace(1)* %1, i64 %654
  %656 = extractelement <16 x i32> %625, i32 10
  %657 = sext i32 %656 to i64
  %658 = getelementptr inbounds float addrspace(1)* %1, i64 %657
  %659 = extractelement <16 x i32> %625, i32 11
  %660 = sext i32 %659 to i64
  %661 = getelementptr inbounds float addrspace(1)* %1, i64 %660
  %662 = extractelement <16 x i32> %625, i32 12
  %663 = sext i32 %662 to i64
  %664 = getelementptr inbounds float addrspace(1)* %1, i64 %663
  %665 = extractelement <16 x i32> %625, i32 13
  %666 = sext i32 %665 to i64
  %667 = getelementptr inbounds float addrspace(1)* %1, i64 %666
  %668 = extractelement <16 x i32> %625, i32 14
  %669 = sext i32 %668 to i64
  %670 = getelementptr inbounds float addrspace(1)* %1, i64 %669
  %671 = extractelement <16 x i32> %625, i32 15
  %672 = sext i32 %671 to i64
  %673 = getelementptr inbounds float addrspace(1)* %1, i64 %672
  store float %extract16.i, float addrspace(1)* %628, align 4
  store float %extract17.i, float addrspace(1)* %631, align 4
  store float %extract18.i, float addrspace(1)* %634, align 4
  store float %extract19.i, float addrspace(1)* %637, align 4
  store float %extract20.i, float addrspace(1)* %640, align 4
  store float %extract21.i, float addrspace(1)* %643, align 4
  store float %extract22.i, float addrspace(1)* %646, align 4
  store float %extract23.i, float addrspace(1)* %649, align 4
  store float %extract24.i, float addrspace(1)* %652, align 4
  store float %extract25.i, float addrspace(1)* %655, align 4
  store float %extract26.i, float addrspace(1)* %658, align 4
  store float %extract27.i, float addrspace(1)* %661, align 4
  store float %extract28.i, float addrspace(1)* %664, align 4
  store float %extract29.i, float addrspace(1)* %667, align 4
  store float %extract30.i, float addrspace(1)* %670, align 4
  store float %extract31.i, float addrspace(1)* %673, align 4
  %674 = add nsw <16 x i32> %vectorPHI.i, <i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13, i32 13>
  %675 = and <16 x i32> %674, %vector.i
  %676 = extractelement <16 x i32> %675, i32 0
  %677 = sext i32 %676 to i64
  %678 = getelementptr inbounds float addrspace(1)* %1, i64 %677
  %679 = extractelement <16 x i32> %675, i32 1
  %680 = sext i32 %679 to i64
  %681 = getelementptr inbounds float addrspace(1)* %1, i64 %680
  %682 = extractelement <16 x i32> %675, i32 2
  %683 = sext i32 %682 to i64
  %684 = getelementptr inbounds float addrspace(1)* %1, i64 %683
  %685 = extractelement <16 x i32> %675, i32 3
  %686 = sext i32 %685 to i64
  %687 = getelementptr inbounds float addrspace(1)* %1, i64 %686
  %688 = extractelement <16 x i32> %675, i32 4
  %689 = sext i32 %688 to i64
  %690 = getelementptr inbounds float addrspace(1)* %1, i64 %689
  %691 = extractelement <16 x i32> %675, i32 5
  %692 = sext i32 %691 to i64
  %693 = getelementptr inbounds float addrspace(1)* %1, i64 %692
  %694 = extractelement <16 x i32> %675, i32 6
  %695 = sext i32 %694 to i64
  %696 = getelementptr inbounds float addrspace(1)* %1, i64 %695
  %697 = extractelement <16 x i32> %675, i32 7
  %698 = sext i32 %697 to i64
  %699 = getelementptr inbounds float addrspace(1)* %1, i64 %698
  %700 = extractelement <16 x i32> %675, i32 8
  %701 = sext i32 %700 to i64
  %702 = getelementptr inbounds float addrspace(1)* %1, i64 %701
  %703 = extractelement <16 x i32> %675, i32 9
  %704 = sext i32 %703 to i64
  %705 = getelementptr inbounds float addrspace(1)* %1, i64 %704
  %706 = extractelement <16 x i32> %675, i32 10
  %707 = sext i32 %706 to i64
  %708 = getelementptr inbounds float addrspace(1)* %1, i64 %707
  %709 = extractelement <16 x i32> %675, i32 11
  %710 = sext i32 %709 to i64
  %711 = getelementptr inbounds float addrspace(1)* %1, i64 %710
  %712 = extractelement <16 x i32> %675, i32 12
  %713 = sext i32 %712 to i64
  %714 = getelementptr inbounds float addrspace(1)* %1, i64 %713
  %715 = extractelement <16 x i32> %675, i32 13
  %716 = sext i32 %715 to i64
  %717 = getelementptr inbounds float addrspace(1)* %1, i64 %716
  %718 = extractelement <16 x i32> %675, i32 14
  %719 = sext i32 %718 to i64
  %720 = getelementptr inbounds float addrspace(1)* %1, i64 %719
  %721 = extractelement <16 x i32> %675, i32 15
  %722 = sext i32 %721 to i64
  %723 = getelementptr inbounds float addrspace(1)* %1, i64 %722
  store float %extract16.i, float addrspace(1)* %678, align 4
  store float %extract17.i, float addrspace(1)* %681, align 4
  store float %extract18.i, float addrspace(1)* %684, align 4
  store float %extract19.i, float addrspace(1)* %687, align 4
  store float %extract20.i, float addrspace(1)* %690, align 4
  store float %extract21.i, float addrspace(1)* %693, align 4
  store float %extract22.i, float addrspace(1)* %696, align 4
  store float %extract23.i, float addrspace(1)* %699, align 4
  store float %extract24.i, float addrspace(1)* %702, align 4
  store float %extract25.i, float addrspace(1)* %705, align 4
  store float %extract26.i, float addrspace(1)* %708, align 4
  store float %extract27.i, float addrspace(1)* %711, align 4
  store float %extract28.i, float addrspace(1)* %714, align 4
  store float %extract29.i, float addrspace(1)* %717, align 4
  store float %extract30.i, float addrspace(1)* %720, align 4
  store float %extract31.i, float addrspace(1)* %723, align 4
  %724 = add nsw <16 x i32> %vectorPHI.i, <i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14, i32 14>
  %725 = and <16 x i32> %724, %vector.i
  %726 = extractelement <16 x i32> %725, i32 0
  %727 = sext i32 %726 to i64
  %728 = getelementptr inbounds float addrspace(1)* %1, i64 %727
  %729 = extractelement <16 x i32> %725, i32 1
  %730 = sext i32 %729 to i64
  %731 = getelementptr inbounds float addrspace(1)* %1, i64 %730
  %732 = extractelement <16 x i32> %725, i32 2
  %733 = sext i32 %732 to i64
  %734 = getelementptr inbounds float addrspace(1)* %1, i64 %733
  %735 = extractelement <16 x i32> %725, i32 3
  %736 = sext i32 %735 to i64
  %737 = getelementptr inbounds float addrspace(1)* %1, i64 %736
  %738 = extractelement <16 x i32> %725, i32 4
  %739 = sext i32 %738 to i64
  %740 = getelementptr inbounds float addrspace(1)* %1, i64 %739
  %741 = extractelement <16 x i32> %725, i32 5
  %742 = sext i32 %741 to i64
  %743 = getelementptr inbounds float addrspace(1)* %1, i64 %742
  %744 = extractelement <16 x i32> %725, i32 6
  %745 = sext i32 %744 to i64
  %746 = getelementptr inbounds float addrspace(1)* %1, i64 %745
  %747 = extractelement <16 x i32> %725, i32 7
  %748 = sext i32 %747 to i64
  %749 = getelementptr inbounds float addrspace(1)* %1, i64 %748
  %750 = extractelement <16 x i32> %725, i32 8
  %751 = sext i32 %750 to i64
  %752 = getelementptr inbounds float addrspace(1)* %1, i64 %751
  %753 = extractelement <16 x i32> %725, i32 9
  %754 = sext i32 %753 to i64
  %755 = getelementptr inbounds float addrspace(1)* %1, i64 %754
  %756 = extractelement <16 x i32> %725, i32 10
  %757 = sext i32 %756 to i64
  %758 = getelementptr inbounds float addrspace(1)* %1, i64 %757
  %759 = extractelement <16 x i32> %725, i32 11
  %760 = sext i32 %759 to i64
  %761 = getelementptr inbounds float addrspace(1)* %1, i64 %760
  %762 = extractelement <16 x i32> %725, i32 12
  %763 = sext i32 %762 to i64
  %764 = getelementptr inbounds float addrspace(1)* %1, i64 %763
  %765 = extractelement <16 x i32> %725, i32 13
  %766 = sext i32 %765 to i64
  %767 = getelementptr inbounds float addrspace(1)* %1, i64 %766
  %768 = extractelement <16 x i32> %725, i32 14
  %769 = sext i32 %768 to i64
  %770 = getelementptr inbounds float addrspace(1)* %1, i64 %769
  %771 = extractelement <16 x i32> %725, i32 15
  %772 = sext i32 %771 to i64
  %773 = getelementptr inbounds float addrspace(1)* %1, i64 %772
  store float %extract16.i, float addrspace(1)* %728, align 4
  store float %extract17.i, float addrspace(1)* %731, align 4
  store float %extract18.i, float addrspace(1)* %734, align 4
  store float %extract19.i, float addrspace(1)* %737, align 4
  store float %extract20.i, float addrspace(1)* %740, align 4
  store float %extract21.i, float addrspace(1)* %743, align 4
  store float %extract22.i, float addrspace(1)* %746, align 4
  store float %extract23.i, float addrspace(1)* %749, align 4
  store float %extract24.i, float addrspace(1)* %752, align 4
  store float %extract25.i, float addrspace(1)* %755, align 4
  store float %extract26.i, float addrspace(1)* %758, align 4
  store float %extract27.i, float addrspace(1)* %761, align 4
  store float %extract28.i, float addrspace(1)* %764, align 4
  store float %extract29.i, float addrspace(1)* %767, align 4
  store float %extract30.i, float addrspace(1)* %770, align 4
  store float %extract31.i, float addrspace(1)* %773, align 4
  %774 = add nsw <16 x i32> %vectorPHI.i, <i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15, i32 15>
  %775 = and <16 x i32> %774, %vector.i
  %776 = extractelement <16 x i32> %775, i32 0
  %777 = sext i32 %776 to i64
  %778 = getelementptr inbounds float addrspace(1)* %1, i64 %777
  %779 = extractelement <16 x i32> %775, i32 1
  %780 = sext i32 %779 to i64
  %781 = getelementptr inbounds float addrspace(1)* %1, i64 %780
  %782 = extractelement <16 x i32> %775, i32 2
  %783 = sext i32 %782 to i64
  %784 = getelementptr inbounds float addrspace(1)* %1, i64 %783
  %785 = extractelement <16 x i32> %775, i32 3
  %786 = sext i32 %785 to i64
  %787 = getelementptr inbounds float addrspace(1)* %1, i64 %786
  %788 = extractelement <16 x i32> %775, i32 4
  %789 = sext i32 %788 to i64
  %790 = getelementptr inbounds float addrspace(1)* %1, i64 %789
  %791 = extractelement <16 x i32> %775, i32 5
  %792 = sext i32 %791 to i64
  %793 = getelementptr inbounds float addrspace(1)* %1, i64 %792
  %794 = extractelement <16 x i32> %775, i32 6
  %795 = sext i32 %794 to i64
  %796 = getelementptr inbounds float addrspace(1)* %1, i64 %795
  %797 = extractelement <16 x i32> %775, i32 7
  %798 = sext i32 %797 to i64
  %799 = getelementptr inbounds float addrspace(1)* %1, i64 %798
  %800 = extractelement <16 x i32> %775, i32 8
  %801 = sext i32 %800 to i64
  %802 = getelementptr inbounds float addrspace(1)* %1, i64 %801
  %803 = extractelement <16 x i32> %775, i32 9
  %804 = sext i32 %803 to i64
  %805 = getelementptr inbounds float addrspace(1)* %1, i64 %804
  %806 = extractelement <16 x i32> %775, i32 10
  %807 = sext i32 %806 to i64
  %808 = getelementptr inbounds float addrspace(1)* %1, i64 %807
  %809 = extractelement <16 x i32> %775, i32 11
  %810 = sext i32 %809 to i64
  %811 = getelementptr inbounds float addrspace(1)* %1, i64 %810
  %812 = extractelement <16 x i32> %775, i32 12
  %813 = sext i32 %812 to i64
  %814 = getelementptr inbounds float addrspace(1)* %1, i64 %813
  %815 = extractelement <16 x i32> %775, i32 13
  %816 = sext i32 %815 to i64
  %817 = getelementptr inbounds float addrspace(1)* %1, i64 %816
  %818 = extractelement <16 x i32> %775, i32 14
  %819 = sext i32 %818 to i64
  %820 = getelementptr inbounds float addrspace(1)* %1, i64 %819
  %821 = extractelement <16 x i32> %775, i32 15
  %822 = sext i32 %821 to i64
  %823 = getelementptr inbounds float addrspace(1)* %1, i64 %822
  store float %extract16.i, float addrspace(1)* %778, align 4
  store float %extract17.i, float addrspace(1)* %781, align 4
  store float %extract18.i, float addrspace(1)* %784, align 4
  store float %extract19.i, float addrspace(1)* %787, align 4
  store float %extract20.i, float addrspace(1)* %790, align 4
  store float %extract21.i, float addrspace(1)* %793, align 4
  store float %extract22.i, float addrspace(1)* %796, align 4
  store float %extract23.i, float addrspace(1)* %799, align 4
  store float %extract24.i, float addrspace(1)* %802, align 4
  store float %extract25.i, float addrspace(1)* %805, align 4
  store float %extract26.i, float addrspace(1)* %808, align 4
  store float %extract27.i, float addrspace(1)* %811, align 4
  store float %extract28.i, float addrspace(1)* %814, align 4
  store float %extract29.i, float addrspace(1)* %817, align 4
  store float %extract30.i, float addrspace(1)* %820, align 4
  store float %extract31.i, float addrspace(1)* %823, align 4
  %824 = add nsw i32 %j.01.i, 1
  %825 = add nsw <16 x i32> %vectorPHI.i, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
  %826 = and <16 x i32> %825, %vector.i
  %exitcond.i = icmp eq i32 %824, 512
  br i1 %exitcond.i, label %._crit_edge.i, label %24

._crit_edge.i:                                    ; preds = %24
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.writeGlobalMemoryUnit_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.writeGlobalMemoryUnit_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeGlobalMemoryUnit_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeGlobalMemoryUnit_locals_anchor", void (i8*)* @writeGlobalMemoryUnit}
!1 = metadata !{i32 0, i32 0, i32 0}
