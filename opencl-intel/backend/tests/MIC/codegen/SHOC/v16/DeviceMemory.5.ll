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

declare void @____Vectorized_.writeGlobalMemoryCoalesced_original(float addrspace(1)* nocapture, i32) nounwind

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

define void @__writeGlobalMemoryCoalesced_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %7 = sitofp i32 %6 to float
  br label %8

; <label>:8                                       ; preds = %8, %SyncBB
  %s.02 = phi i32 [ %6, %SyncBB ], [ %74, %8 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %72, %8 ]
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
  br i1 %check.WI.iter, label %thenBB, label %SyncBB3

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB3:                                          ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.writeGlobalMemoryCoalesced_separated_args(float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
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
  %8 = sitofp <16 x i32> %7 to <16 x float>
  %extract16 = extractelement <16 x float> %8, i32 0
  %extract17 = extractelement <16 x float> %8, i32 1
  %extract18 = extractelement <16 x float> %8, i32 2
  %extract19 = extractelement <16 x float> %8, i32 3
  %extract20 = extractelement <16 x float> %8, i32 4
  %extract21 = extractelement <16 x float> %8, i32 5
  %extract22 = extractelement <16 x float> %8, i32 6
  %extract23 = extractelement <16 x float> %8, i32 7
  %extract24 = extractelement <16 x float> %8, i32 8
  %extract25 = extractelement <16 x float> %8, i32 9
  %extract26 = extractelement <16 x float> %8, i32 10
  %extract27 = extractelement <16 x float> %8, i32 11
  %extract28 = extractelement <16 x float> %8, i32 12
  %extract29 = extractelement <16 x float> %8, i32 13
  %extract30 = extractelement <16 x float> %8, i32 14
  %extract31 = extractelement <16 x float> %8, i32 15
  br label %9

; <label>:9                                       ; preds = %9, %SyncBB
  %vectorPHI = phi <16 x i32> [ %7, %SyncBB ], [ %811, %9 ]
  %j.01 = phi i32 [ 0, %SyncBB ], [ %809, %9 ]
  %10 = and <16 x i32> %vectorPHI, %vector
  %11 = extractelement <16 x i32> %10, i32 0
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds float addrspace(1)* %output, i64 %12
  %14 = extractelement <16 x i32> %10, i32 1
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds float addrspace(1)* %output, i64 %15
  %17 = extractelement <16 x i32> %10, i32 2
  %18 = sext i32 %17 to i64
  %19 = getelementptr inbounds float addrspace(1)* %output, i64 %18
  %20 = extractelement <16 x i32> %10, i32 3
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds float addrspace(1)* %output, i64 %21
  %23 = extractelement <16 x i32> %10, i32 4
  %24 = sext i32 %23 to i64
  %25 = getelementptr inbounds float addrspace(1)* %output, i64 %24
  %26 = extractelement <16 x i32> %10, i32 5
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds float addrspace(1)* %output, i64 %27
  %29 = extractelement <16 x i32> %10, i32 6
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds float addrspace(1)* %output, i64 %30
  %32 = extractelement <16 x i32> %10, i32 7
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds float addrspace(1)* %output, i64 %33
  %35 = extractelement <16 x i32> %10, i32 8
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds float addrspace(1)* %output, i64 %36
  %38 = extractelement <16 x i32> %10, i32 9
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds float addrspace(1)* %output, i64 %39
  %41 = extractelement <16 x i32> %10, i32 10
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds float addrspace(1)* %output, i64 %42
  %44 = extractelement <16 x i32> %10, i32 11
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds float addrspace(1)* %output, i64 %45
  %47 = extractelement <16 x i32> %10, i32 12
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds float addrspace(1)* %output, i64 %48
  %50 = extractelement <16 x i32> %10, i32 13
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds float addrspace(1)* %output, i64 %51
  %53 = extractelement <16 x i32> %10, i32 14
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds float addrspace(1)* %output, i64 %54
  %56 = extractelement <16 x i32> %10, i32 15
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float addrspace(1)* %output, i64 %57
  store float %extract16, float addrspace(1)* %13, align 4
  store float %extract17, float addrspace(1)* %16, align 4
  store float %extract18, float addrspace(1)* %19, align 4
  store float %extract19, float addrspace(1)* %22, align 4
  store float %extract20, float addrspace(1)* %25, align 4
  store float %extract21, float addrspace(1)* %28, align 4
  store float %extract22, float addrspace(1)* %31, align 4
  store float %extract23, float addrspace(1)* %34, align 4
  store float %extract24, float addrspace(1)* %37, align 4
  store float %extract25, float addrspace(1)* %40, align 4
  store float %extract26, float addrspace(1)* %43, align 4
  store float %extract27, float addrspace(1)* %46, align 4
  store float %extract28, float addrspace(1)* %49, align 4
  store float %extract29, float addrspace(1)* %52, align 4
  store float %extract30, float addrspace(1)* %55, align 4
  store float %extract31, float addrspace(1)* %58, align 4
  %59 = add nsw <16 x i32> %vectorPHI, <i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32>
  %60 = and <16 x i32> %59, %vector
  %61 = extractelement <16 x i32> %60, i32 0
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %output, i64 %62
  %64 = extractelement <16 x i32> %60, i32 1
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %output, i64 %65
  %67 = extractelement <16 x i32> %60, i32 2
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds float addrspace(1)* %output, i64 %68
  %70 = extractelement <16 x i32> %60, i32 3
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %output, i64 %71
  %73 = extractelement <16 x i32> %60, i32 4
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds float addrspace(1)* %output, i64 %74
  %76 = extractelement <16 x i32> %60, i32 5
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %output, i64 %77
  %79 = extractelement <16 x i32> %60, i32 6
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %output, i64 %80
  %82 = extractelement <16 x i32> %60, i32 7
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds float addrspace(1)* %output, i64 %83
  %85 = extractelement <16 x i32> %60, i32 8
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds float addrspace(1)* %output, i64 %86
  %88 = extractelement <16 x i32> %60, i32 9
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds float addrspace(1)* %output, i64 %89
  %91 = extractelement <16 x i32> %60, i32 10
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds float addrspace(1)* %output, i64 %92
  %94 = extractelement <16 x i32> %60, i32 11
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds float addrspace(1)* %output, i64 %95
  %97 = extractelement <16 x i32> %60, i32 12
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds float addrspace(1)* %output, i64 %98
  %100 = extractelement <16 x i32> %60, i32 13
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds float addrspace(1)* %output, i64 %101
  %103 = extractelement <16 x i32> %60, i32 14
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds float addrspace(1)* %output, i64 %104
  %106 = extractelement <16 x i32> %60, i32 15
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds float addrspace(1)* %output, i64 %107
  store float %extract16, float addrspace(1)* %63, align 4
  store float %extract17, float addrspace(1)* %66, align 4
  store float %extract18, float addrspace(1)* %69, align 4
  store float %extract19, float addrspace(1)* %72, align 4
  store float %extract20, float addrspace(1)* %75, align 4
  store float %extract21, float addrspace(1)* %78, align 4
  store float %extract22, float addrspace(1)* %81, align 4
  store float %extract23, float addrspace(1)* %84, align 4
  store float %extract24, float addrspace(1)* %87, align 4
  store float %extract25, float addrspace(1)* %90, align 4
  store float %extract26, float addrspace(1)* %93, align 4
  store float %extract27, float addrspace(1)* %96, align 4
  store float %extract28, float addrspace(1)* %99, align 4
  store float %extract29, float addrspace(1)* %102, align 4
  store float %extract30, float addrspace(1)* %105, align 4
  store float %extract31, float addrspace(1)* %108, align 4
  %109 = add nsw <16 x i32> %vectorPHI, <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>
  %110 = and <16 x i32> %109, %vector
  %111 = extractelement <16 x i32> %110, i32 0
  %112 = sext i32 %111 to i64
  %113 = getelementptr inbounds float addrspace(1)* %output, i64 %112
  %114 = extractelement <16 x i32> %110, i32 1
  %115 = sext i32 %114 to i64
  %116 = getelementptr inbounds float addrspace(1)* %output, i64 %115
  %117 = extractelement <16 x i32> %110, i32 2
  %118 = sext i32 %117 to i64
  %119 = getelementptr inbounds float addrspace(1)* %output, i64 %118
  %120 = extractelement <16 x i32> %110, i32 3
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds float addrspace(1)* %output, i64 %121
  %123 = extractelement <16 x i32> %110, i32 4
  %124 = sext i32 %123 to i64
  %125 = getelementptr inbounds float addrspace(1)* %output, i64 %124
  %126 = extractelement <16 x i32> %110, i32 5
  %127 = sext i32 %126 to i64
  %128 = getelementptr inbounds float addrspace(1)* %output, i64 %127
  %129 = extractelement <16 x i32> %110, i32 6
  %130 = sext i32 %129 to i64
  %131 = getelementptr inbounds float addrspace(1)* %output, i64 %130
  %132 = extractelement <16 x i32> %110, i32 7
  %133 = sext i32 %132 to i64
  %134 = getelementptr inbounds float addrspace(1)* %output, i64 %133
  %135 = extractelement <16 x i32> %110, i32 8
  %136 = sext i32 %135 to i64
  %137 = getelementptr inbounds float addrspace(1)* %output, i64 %136
  %138 = extractelement <16 x i32> %110, i32 9
  %139 = sext i32 %138 to i64
  %140 = getelementptr inbounds float addrspace(1)* %output, i64 %139
  %141 = extractelement <16 x i32> %110, i32 10
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds float addrspace(1)* %output, i64 %142
  %144 = extractelement <16 x i32> %110, i32 11
  %145 = sext i32 %144 to i64
  %146 = getelementptr inbounds float addrspace(1)* %output, i64 %145
  %147 = extractelement <16 x i32> %110, i32 12
  %148 = sext i32 %147 to i64
  %149 = getelementptr inbounds float addrspace(1)* %output, i64 %148
  %150 = extractelement <16 x i32> %110, i32 13
  %151 = sext i32 %150 to i64
  %152 = getelementptr inbounds float addrspace(1)* %output, i64 %151
  %153 = extractelement <16 x i32> %110, i32 14
  %154 = sext i32 %153 to i64
  %155 = getelementptr inbounds float addrspace(1)* %output, i64 %154
  %156 = extractelement <16 x i32> %110, i32 15
  %157 = sext i32 %156 to i64
  %158 = getelementptr inbounds float addrspace(1)* %output, i64 %157
  store float %extract16, float addrspace(1)* %113, align 4
  store float %extract17, float addrspace(1)* %116, align 4
  store float %extract18, float addrspace(1)* %119, align 4
  store float %extract19, float addrspace(1)* %122, align 4
  store float %extract20, float addrspace(1)* %125, align 4
  store float %extract21, float addrspace(1)* %128, align 4
  store float %extract22, float addrspace(1)* %131, align 4
  store float %extract23, float addrspace(1)* %134, align 4
  store float %extract24, float addrspace(1)* %137, align 4
  store float %extract25, float addrspace(1)* %140, align 4
  store float %extract26, float addrspace(1)* %143, align 4
  store float %extract27, float addrspace(1)* %146, align 4
  store float %extract28, float addrspace(1)* %149, align 4
  store float %extract29, float addrspace(1)* %152, align 4
  store float %extract30, float addrspace(1)* %155, align 4
  store float %extract31, float addrspace(1)* %158, align 4
  %159 = add nsw <16 x i32> %vectorPHI, <i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96>
  %160 = and <16 x i32> %159, %vector
  %161 = extractelement <16 x i32> %160, i32 0
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds float addrspace(1)* %output, i64 %162
  %164 = extractelement <16 x i32> %160, i32 1
  %165 = sext i32 %164 to i64
  %166 = getelementptr inbounds float addrspace(1)* %output, i64 %165
  %167 = extractelement <16 x i32> %160, i32 2
  %168 = sext i32 %167 to i64
  %169 = getelementptr inbounds float addrspace(1)* %output, i64 %168
  %170 = extractelement <16 x i32> %160, i32 3
  %171 = sext i32 %170 to i64
  %172 = getelementptr inbounds float addrspace(1)* %output, i64 %171
  %173 = extractelement <16 x i32> %160, i32 4
  %174 = sext i32 %173 to i64
  %175 = getelementptr inbounds float addrspace(1)* %output, i64 %174
  %176 = extractelement <16 x i32> %160, i32 5
  %177 = sext i32 %176 to i64
  %178 = getelementptr inbounds float addrspace(1)* %output, i64 %177
  %179 = extractelement <16 x i32> %160, i32 6
  %180 = sext i32 %179 to i64
  %181 = getelementptr inbounds float addrspace(1)* %output, i64 %180
  %182 = extractelement <16 x i32> %160, i32 7
  %183 = sext i32 %182 to i64
  %184 = getelementptr inbounds float addrspace(1)* %output, i64 %183
  %185 = extractelement <16 x i32> %160, i32 8
  %186 = sext i32 %185 to i64
  %187 = getelementptr inbounds float addrspace(1)* %output, i64 %186
  %188 = extractelement <16 x i32> %160, i32 9
  %189 = sext i32 %188 to i64
  %190 = getelementptr inbounds float addrspace(1)* %output, i64 %189
  %191 = extractelement <16 x i32> %160, i32 10
  %192 = sext i32 %191 to i64
  %193 = getelementptr inbounds float addrspace(1)* %output, i64 %192
  %194 = extractelement <16 x i32> %160, i32 11
  %195 = sext i32 %194 to i64
  %196 = getelementptr inbounds float addrspace(1)* %output, i64 %195
  %197 = extractelement <16 x i32> %160, i32 12
  %198 = sext i32 %197 to i64
  %199 = getelementptr inbounds float addrspace(1)* %output, i64 %198
  %200 = extractelement <16 x i32> %160, i32 13
  %201 = sext i32 %200 to i64
  %202 = getelementptr inbounds float addrspace(1)* %output, i64 %201
  %203 = extractelement <16 x i32> %160, i32 14
  %204 = sext i32 %203 to i64
  %205 = getelementptr inbounds float addrspace(1)* %output, i64 %204
  %206 = extractelement <16 x i32> %160, i32 15
  %207 = sext i32 %206 to i64
  %208 = getelementptr inbounds float addrspace(1)* %output, i64 %207
  store float %extract16, float addrspace(1)* %163, align 4
  store float %extract17, float addrspace(1)* %166, align 4
  store float %extract18, float addrspace(1)* %169, align 4
  store float %extract19, float addrspace(1)* %172, align 4
  store float %extract20, float addrspace(1)* %175, align 4
  store float %extract21, float addrspace(1)* %178, align 4
  store float %extract22, float addrspace(1)* %181, align 4
  store float %extract23, float addrspace(1)* %184, align 4
  store float %extract24, float addrspace(1)* %187, align 4
  store float %extract25, float addrspace(1)* %190, align 4
  store float %extract26, float addrspace(1)* %193, align 4
  store float %extract27, float addrspace(1)* %196, align 4
  store float %extract28, float addrspace(1)* %199, align 4
  store float %extract29, float addrspace(1)* %202, align 4
  store float %extract30, float addrspace(1)* %205, align 4
  store float %extract31, float addrspace(1)* %208, align 4
  %209 = add nsw <16 x i32> %vectorPHI, <i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128>
  %210 = and <16 x i32> %209, %vector
  %211 = extractelement <16 x i32> %210, i32 0
  %212 = sext i32 %211 to i64
  %213 = getelementptr inbounds float addrspace(1)* %output, i64 %212
  %214 = extractelement <16 x i32> %210, i32 1
  %215 = sext i32 %214 to i64
  %216 = getelementptr inbounds float addrspace(1)* %output, i64 %215
  %217 = extractelement <16 x i32> %210, i32 2
  %218 = sext i32 %217 to i64
  %219 = getelementptr inbounds float addrspace(1)* %output, i64 %218
  %220 = extractelement <16 x i32> %210, i32 3
  %221 = sext i32 %220 to i64
  %222 = getelementptr inbounds float addrspace(1)* %output, i64 %221
  %223 = extractelement <16 x i32> %210, i32 4
  %224 = sext i32 %223 to i64
  %225 = getelementptr inbounds float addrspace(1)* %output, i64 %224
  %226 = extractelement <16 x i32> %210, i32 5
  %227 = sext i32 %226 to i64
  %228 = getelementptr inbounds float addrspace(1)* %output, i64 %227
  %229 = extractelement <16 x i32> %210, i32 6
  %230 = sext i32 %229 to i64
  %231 = getelementptr inbounds float addrspace(1)* %output, i64 %230
  %232 = extractelement <16 x i32> %210, i32 7
  %233 = sext i32 %232 to i64
  %234 = getelementptr inbounds float addrspace(1)* %output, i64 %233
  %235 = extractelement <16 x i32> %210, i32 8
  %236 = sext i32 %235 to i64
  %237 = getelementptr inbounds float addrspace(1)* %output, i64 %236
  %238 = extractelement <16 x i32> %210, i32 9
  %239 = sext i32 %238 to i64
  %240 = getelementptr inbounds float addrspace(1)* %output, i64 %239
  %241 = extractelement <16 x i32> %210, i32 10
  %242 = sext i32 %241 to i64
  %243 = getelementptr inbounds float addrspace(1)* %output, i64 %242
  %244 = extractelement <16 x i32> %210, i32 11
  %245 = sext i32 %244 to i64
  %246 = getelementptr inbounds float addrspace(1)* %output, i64 %245
  %247 = extractelement <16 x i32> %210, i32 12
  %248 = sext i32 %247 to i64
  %249 = getelementptr inbounds float addrspace(1)* %output, i64 %248
  %250 = extractelement <16 x i32> %210, i32 13
  %251 = sext i32 %250 to i64
  %252 = getelementptr inbounds float addrspace(1)* %output, i64 %251
  %253 = extractelement <16 x i32> %210, i32 14
  %254 = sext i32 %253 to i64
  %255 = getelementptr inbounds float addrspace(1)* %output, i64 %254
  %256 = extractelement <16 x i32> %210, i32 15
  %257 = sext i32 %256 to i64
  %258 = getelementptr inbounds float addrspace(1)* %output, i64 %257
  store float %extract16, float addrspace(1)* %213, align 4
  store float %extract17, float addrspace(1)* %216, align 4
  store float %extract18, float addrspace(1)* %219, align 4
  store float %extract19, float addrspace(1)* %222, align 4
  store float %extract20, float addrspace(1)* %225, align 4
  store float %extract21, float addrspace(1)* %228, align 4
  store float %extract22, float addrspace(1)* %231, align 4
  store float %extract23, float addrspace(1)* %234, align 4
  store float %extract24, float addrspace(1)* %237, align 4
  store float %extract25, float addrspace(1)* %240, align 4
  store float %extract26, float addrspace(1)* %243, align 4
  store float %extract27, float addrspace(1)* %246, align 4
  store float %extract28, float addrspace(1)* %249, align 4
  store float %extract29, float addrspace(1)* %252, align 4
  store float %extract30, float addrspace(1)* %255, align 4
  store float %extract31, float addrspace(1)* %258, align 4
  %259 = add nsw <16 x i32> %vectorPHI, <i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160>
  %260 = and <16 x i32> %259, %vector
  %261 = extractelement <16 x i32> %260, i32 0
  %262 = sext i32 %261 to i64
  %263 = getelementptr inbounds float addrspace(1)* %output, i64 %262
  %264 = extractelement <16 x i32> %260, i32 1
  %265 = sext i32 %264 to i64
  %266 = getelementptr inbounds float addrspace(1)* %output, i64 %265
  %267 = extractelement <16 x i32> %260, i32 2
  %268 = sext i32 %267 to i64
  %269 = getelementptr inbounds float addrspace(1)* %output, i64 %268
  %270 = extractelement <16 x i32> %260, i32 3
  %271 = sext i32 %270 to i64
  %272 = getelementptr inbounds float addrspace(1)* %output, i64 %271
  %273 = extractelement <16 x i32> %260, i32 4
  %274 = sext i32 %273 to i64
  %275 = getelementptr inbounds float addrspace(1)* %output, i64 %274
  %276 = extractelement <16 x i32> %260, i32 5
  %277 = sext i32 %276 to i64
  %278 = getelementptr inbounds float addrspace(1)* %output, i64 %277
  %279 = extractelement <16 x i32> %260, i32 6
  %280 = sext i32 %279 to i64
  %281 = getelementptr inbounds float addrspace(1)* %output, i64 %280
  %282 = extractelement <16 x i32> %260, i32 7
  %283 = sext i32 %282 to i64
  %284 = getelementptr inbounds float addrspace(1)* %output, i64 %283
  %285 = extractelement <16 x i32> %260, i32 8
  %286 = sext i32 %285 to i64
  %287 = getelementptr inbounds float addrspace(1)* %output, i64 %286
  %288 = extractelement <16 x i32> %260, i32 9
  %289 = sext i32 %288 to i64
  %290 = getelementptr inbounds float addrspace(1)* %output, i64 %289
  %291 = extractelement <16 x i32> %260, i32 10
  %292 = sext i32 %291 to i64
  %293 = getelementptr inbounds float addrspace(1)* %output, i64 %292
  %294 = extractelement <16 x i32> %260, i32 11
  %295 = sext i32 %294 to i64
  %296 = getelementptr inbounds float addrspace(1)* %output, i64 %295
  %297 = extractelement <16 x i32> %260, i32 12
  %298 = sext i32 %297 to i64
  %299 = getelementptr inbounds float addrspace(1)* %output, i64 %298
  %300 = extractelement <16 x i32> %260, i32 13
  %301 = sext i32 %300 to i64
  %302 = getelementptr inbounds float addrspace(1)* %output, i64 %301
  %303 = extractelement <16 x i32> %260, i32 14
  %304 = sext i32 %303 to i64
  %305 = getelementptr inbounds float addrspace(1)* %output, i64 %304
  %306 = extractelement <16 x i32> %260, i32 15
  %307 = sext i32 %306 to i64
  %308 = getelementptr inbounds float addrspace(1)* %output, i64 %307
  store float %extract16, float addrspace(1)* %263, align 4
  store float %extract17, float addrspace(1)* %266, align 4
  store float %extract18, float addrspace(1)* %269, align 4
  store float %extract19, float addrspace(1)* %272, align 4
  store float %extract20, float addrspace(1)* %275, align 4
  store float %extract21, float addrspace(1)* %278, align 4
  store float %extract22, float addrspace(1)* %281, align 4
  store float %extract23, float addrspace(1)* %284, align 4
  store float %extract24, float addrspace(1)* %287, align 4
  store float %extract25, float addrspace(1)* %290, align 4
  store float %extract26, float addrspace(1)* %293, align 4
  store float %extract27, float addrspace(1)* %296, align 4
  store float %extract28, float addrspace(1)* %299, align 4
  store float %extract29, float addrspace(1)* %302, align 4
  store float %extract30, float addrspace(1)* %305, align 4
  store float %extract31, float addrspace(1)* %308, align 4
  %309 = add nsw <16 x i32> %vectorPHI, <i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192>
  %310 = and <16 x i32> %309, %vector
  %311 = extractelement <16 x i32> %310, i32 0
  %312 = sext i32 %311 to i64
  %313 = getelementptr inbounds float addrspace(1)* %output, i64 %312
  %314 = extractelement <16 x i32> %310, i32 1
  %315 = sext i32 %314 to i64
  %316 = getelementptr inbounds float addrspace(1)* %output, i64 %315
  %317 = extractelement <16 x i32> %310, i32 2
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds float addrspace(1)* %output, i64 %318
  %320 = extractelement <16 x i32> %310, i32 3
  %321 = sext i32 %320 to i64
  %322 = getelementptr inbounds float addrspace(1)* %output, i64 %321
  %323 = extractelement <16 x i32> %310, i32 4
  %324 = sext i32 %323 to i64
  %325 = getelementptr inbounds float addrspace(1)* %output, i64 %324
  %326 = extractelement <16 x i32> %310, i32 5
  %327 = sext i32 %326 to i64
  %328 = getelementptr inbounds float addrspace(1)* %output, i64 %327
  %329 = extractelement <16 x i32> %310, i32 6
  %330 = sext i32 %329 to i64
  %331 = getelementptr inbounds float addrspace(1)* %output, i64 %330
  %332 = extractelement <16 x i32> %310, i32 7
  %333 = sext i32 %332 to i64
  %334 = getelementptr inbounds float addrspace(1)* %output, i64 %333
  %335 = extractelement <16 x i32> %310, i32 8
  %336 = sext i32 %335 to i64
  %337 = getelementptr inbounds float addrspace(1)* %output, i64 %336
  %338 = extractelement <16 x i32> %310, i32 9
  %339 = sext i32 %338 to i64
  %340 = getelementptr inbounds float addrspace(1)* %output, i64 %339
  %341 = extractelement <16 x i32> %310, i32 10
  %342 = sext i32 %341 to i64
  %343 = getelementptr inbounds float addrspace(1)* %output, i64 %342
  %344 = extractelement <16 x i32> %310, i32 11
  %345 = sext i32 %344 to i64
  %346 = getelementptr inbounds float addrspace(1)* %output, i64 %345
  %347 = extractelement <16 x i32> %310, i32 12
  %348 = sext i32 %347 to i64
  %349 = getelementptr inbounds float addrspace(1)* %output, i64 %348
  %350 = extractelement <16 x i32> %310, i32 13
  %351 = sext i32 %350 to i64
  %352 = getelementptr inbounds float addrspace(1)* %output, i64 %351
  %353 = extractelement <16 x i32> %310, i32 14
  %354 = sext i32 %353 to i64
  %355 = getelementptr inbounds float addrspace(1)* %output, i64 %354
  %356 = extractelement <16 x i32> %310, i32 15
  %357 = sext i32 %356 to i64
  %358 = getelementptr inbounds float addrspace(1)* %output, i64 %357
  store float %extract16, float addrspace(1)* %313, align 4
  store float %extract17, float addrspace(1)* %316, align 4
  store float %extract18, float addrspace(1)* %319, align 4
  store float %extract19, float addrspace(1)* %322, align 4
  store float %extract20, float addrspace(1)* %325, align 4
  store float %extract21, float addrspace(1)* %328, align 4
  store float %extract22, float addrspace(1)* %331, align 4
  store float %extract23, float addrspace(1)* %334, align 4
  store float %extract24, float addrspace(1)* %337, align 4
  store float %extract25, float addrspace(1)* %340, align 4
  store float %extract26, float addrspace(1)* %343, align 4
  store float %extract27, float addrspace(1)* %346, align 4
  store float %extract28, float addrspace(1)* %349, align 4
  store float %extract29, float addrspace(1)* %352, align 4
  store float %extract30, float addrspace(1)* %355, align 4
  store float %extract31, float addrspace(1)* %358, align 4
  %359 = add nsw <16 x i32> %vectorPHI, <i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224>
  %360 = and <16 x i32> %359, %vector
  %361 = extractelement <16 x i32> %360, i32 0
  %362 = sext i32 %361 to i64
  %363 = getelementptr inbounds float addrspace(1)* %output, i64 %362
  %364 = extractelement <16 x i32> %360, i32 1
  %365 = sext i32 %364 to i64
  %366 = getelementptr inbounds float addrspace(1)* %output, i64 %365
  %367 = extractelement <16 x i32> %360, i32 2
  %368 = sext i32 %367 to i64
  %369 = getelementptr inbounds float addrspace(1)* %output, i64 %368
  %370 = extractelement <16 x i32> %360, i32 3
  %371 = sext i32 %370 to i64
  %372 = getelementptr inbounds float addrspace(1)* %output, i64 %371
  %373 = extractelement <16 x i32> %360, i32 4
  %374 = sext i32 %373 to i64
  %375 = getelementptr inbounds float addrspace(1)* %output, i64 %374
  %376 = extractelement <16 x i32> %360, i32 5
  %377 = sext i32 %376 to i64
  %378 = getelementptr inbounds float addrspace(1)* %output, i64 %377
  %379 = extractelement <16 x i32> %360, i32 6
  %380 = sext i32 %379 to i64
  %381 = getelementptr inbounds float addrspace(1)* %output, i64 %380
  %382 = extractelement <16 x i32> %360, i32 7
  %383 = sext i32 %382 to i64
  %384 = getelementptr inbounds float addrspace(1)* %output, i64 %383
  %385 = extractelement <16 x i32> %360, i32 8
  %386 = sext i32 %385 to i64
  %387 = getelementptr inbounds float addrspace(1)* %output, i64 %386
  %388 = extractelement <16 x i32> %360, i32 9
  %389 = sext i32 %388 to i64
  %390 = getelementptr inbounds float addrspace(1)* %output, i64 %389
  %391 = extractelement <16 x i32> %360, i32 10
  %392 = sext i32 %391 to i64
  %393 = getelementptr inbounds float addrspace(1)* %output, i64 %392
  %394 = extractelement <16 x i32> %360, i32 11
  %395 = sext i32 %394 to i64
  %396 = getelementptr inbounds float addrspace(1)* %output, i64 %395
  %397 = extractelement <16 x i32> %360, i32 12
  %398 = sext i32 %397 to i64
  %399 = getelementptr inbounds float addrspace(1)* %output, i64 %398
  %400 = extractelement <16 x i32> %360, i32 13
  %401 = sext i32 %400 to i64
  %402 = getelementptr inbounds float addrspace(1)* %output, i64 %401
  %403 = extractelement <16 x i32> %360, i32 14
  %404 = sext i32 %403 to i64
  %405 = getelementptr inbounds float addrspace(1)* %output, i64 %404
  %406 = extractelement <16 x i32> %360, i32 15
  %407 = sext i32 %406 to i64
  %408 = getelementptr inbounds float addrspace(1)* %output, i64 %407
  store float %extract16, float addrspace(1)* %363, align 4
  store float %extract17, float addrspace(1)* %366, align 4
  store float %extract18, float addrspace(1)* %369, align 4
  store float %extract19, float addrspace(1)* %372, align 4
  store float %extract20, float addrspace(1)* %375, align 4
  store float %extract21, float addrspace(1)* %378, align 4
  store float %extract22, float addrspace(1)* %381, align 4
  store float %extract23, float addrspace(1)* %384, align 4
  store float %extract24, float addrspace(1)* %387, align 4
  store float %extract25, float addrspace(1)* %390, align 4
  store float %extract26, float addrspace(1)* %393, align 4
  store float %extract27, float addrspace(1)* %396, align 4
  store float %extract28, float addrspace(1)* %399, align 4
  store float %extract29, float addrspace(1)* %402, align 4
  store float %extract30, float addrspace(1)* %405, align 4
  store float %extract31, float addrspace(1)* %408, align 4
  %409 = add nsw <16 x i32> %vectorPHI, <i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256>
  %410 = and <16 x i32> %409, %vector
  %411 = extractelement <16 x i32> %410, i32 0
  %412 = sext i32 %411 to i64
  %413 = getelementptr inbounds float addrspace(1)* %output, i64 %412
  %414 = extractelement <16 x i32> %410, i32 1
  %415 = sext i32 %414 to i64
  %416 = getelementptr inbounds float addrspace(1)* %output, i64 %415
  %417 = extractelement <16 x i32> %410, i32 2
  %418 = sext i32 %417 to i64
  %419 = getelementptr inbounds float addrspace(1)* %output, i64 %418
  %420 = extractelement <16 x i32> %410, i32 3
  %421 = sext i32 %420 to i64
  %422 = getelementptr inbounds float addrspace(1)* %output, i64 %421
  %423 = extractelement <16 x i32> %410, i32 4
  %424 = sext i32 %423 to i64
  %425 = getelementptr inbounds float addrspace(1)* %output, i64 %424
  %426 = extractelement <16 x i32> %410, i32 5
  %427 = sext i32 %426 to i64
  %428 = getelementptr inbounds float addrspace(1)* %output, i64 %427
  %429 = extractelement <16 x i32> %410, i32 6
  %430 = sext i32 %429 to i64
  %431 = getelementptr inbounds float addrspace(1)* %output, i64 %430
  %432 = extractelement <16 x i32> %410, i32 7
  %433 = sext i32 %432 to i64
  %434 = getelementptr inbounds float addrspace(1)* %output, i64 %433
  %435 = extractelement <16 x i32> %410, i32 8
  %436 = sext i32 %435 to i64
  %437 = getelementptr inbounds float addrspace(1)* %output, i64 %436
  %438 = extractelement <16 x i32> %410, i32 9
  %439 = sext i32 %438 to i64
  %440 = getelementptr inbounds float addrspace(1)* %output, i64 %439
  %441 = extractelement <16 x i32> %410, i32 10
  %442 = sext i32 %441 to i64
  %443 = getelementptr inbounds float addrspace(1)* %output, i64 %442
  %444 = extractelement <16 x i32> %410, i32 11
  %445 = sext i32 %444 to i64
  %446 = getelementptr inbounds float addrspace(1)* %output, i64 %445
  %447 = extractelement <16 x i32> %410, i32 12
  %448 = sext i32 %447 to i64
  %449 = getelementptr inbounds float addrspace(1)* %output, i64 %448
  %450 = extractelement <16 x i32> %410, i32 13
  %451 = sext i32 %450 to i64
  %452 = getelementptr inbounds float addrspace(1)* %output, i64 %451
  %453 = extractelement <16 x i32> %410, i32 14
  %454 = sext i32 %453 to i64
  %455 = getelementptr inbounds float addrspace(1)* %output, i64 %454
  %456 = extractelement <16 x i32> %410, i32 15
  %457 = sext i32 %456 to i64
  %458 = getelementptr inbounds float addrspace(1)* %output, i64 %457
  store float %extract16, float addrspace(1)* %413, align 4
  store float %extract17, float addrspace(1)* %416, align 4
  store float %extract18, float addrspace(1)* %419, align 4
  store float %extract19, float addrspace(1)* %422, align 4
  store float %extract20, float addrspace(1)* %425, align 4
  store float %extract21, float addrspace(1)* %428, align 4
  store float %extract22, float addrspace(1)* %431, align 4
  store float %extract23, float addrspace(1)* %434, align 4
  store float %extract24, float addrspace(1)* %437, align 4
  store float %extract25, float addrspace(1)* %440, align 4
  store float %extract26, float addrspace(1)* %443, align 4
  store float %extract27, float addrspace(1)* %446, align 4
  store float %extract28, float addrspace(1)* %449, align 4
  store float %extract29, float addrspace(1)* %452, align 4
  store float %extract30, float addrspace(1)* %455, align 4
  store float %extract31, float addrspace(1)* %458, align 4
  %459 = add nsw <16 x i32> %vectorPHI, <i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288>
  %460 = and <16 x i32> %459, %vector
  %461 = extractelement <16 x i32> %460, i32 0
  %462 = sext i32 %461 to i64
  %463 = getelementptr inbounds float addrspace(1)* %output, i64 %462
  %464 = extractelement <16 x i32> %460, i32 1
  %465 = sext i32 %464 to i64
  %466 = getelementptr inbounds float addrspace(1)* %output, i64 %465
  %467 = extractelement <16 x i32> %460, i32 2
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds float addrspace(1)* %output, i64 %468
  %470 = extractelement <16 x i32> %460, i32 3
  %471 = sext i32 %470 to i64
  %472 = getelementptr inbounds float addrspace(1)* %output, i64 %471
  %473 = extractelement <16 x i32> %460, i32 4
  %474 = sext i32 %473 to i64
  %475 = getelementptr inbounds float addrspace(1)* %output, i64 %474
  %476 = extractelement <16 x i32> %460, i32 5
  %477 = sext i32 %476 to i64
  %478 = getelementptr inbounds float addrspace(1)* %output, i64 %477
  %479 = extractelement <16 x i32> %460, i32 6
  %480 = sext i32 %479 to i64
  %481 = getelementptr inbounds float addrspace(1)* %output, i64 %480
  %482 = extractelement <16 x i32> %460, i32 7
  %483 = sext i32 %482 to i64
  %484 = getelementptr inbounds float addrspace(1)* %output, i64 %483
  %485 = extractelement <16 x i32> %460, i32 8
  %486 = sext i32 %485 to i64
  %487 = getelementptr inbounds float addrspace(1)* %output, i64 %486
  %488 = extractelement <16 x i32> %460, i32 9
  %489 = sext i32 %488 to i64
  %490 = getelementptr inbounds float addrspace(1)* %output, i64 %489
  %491 = extractelement <16 x i32> %460, i32 10
  %492 = sext i32 %491 to i64
  %493 = getelementptr inbounds float addrspace(1)* %output, i64 %492
  %494 = extractelement <16 x i32> %460, i32 11
  %495 = sext i32 %494 to i64
  %496 = getelementptr inbounds float addrspace(1)* %output, i64 %495
  %497 = extractelement <16 x i32> %460, i32 12
  %498 = sext i32 %497 to i64
  %499 = getelementptr inbounds float addrspace(1)* %output, i64 %498
  %500 = extractelement <16 x i32> %460, i32 13
  %501 = sext i32 %500 to i64
  %502 = getelementptr inbounds float addrspace(1)* %output, i64 %501
  %503 = extractelement <16 x i32> %460, i32 14
  %504 = sext i32 %503 to i64
  %505 = getelementptr inbounds float addrspace(1)* %output, i64 %504
  %506 = extractelement <16 x i32> %460, i32 15
  %507 = sext i32 %506 to i64
  %508 = getelementptr inbounds float addrspace(1)* %output, i64 %507
  store float %extract16, float addrspace(1)* %463, align 4
  store float %extract17, float addrspace(1)* %466, align 4
  store float %extract18, float addrspace(1)* %469, align 4
  store float %extract19, float addrspace(1)* %472, align 4
  store float %extract20, float addrspace(1)* %475, align 4
  store float %extract21, float addrspace(1)* %478, align 4
  store float %extract22, float addrspace(1)* %481, align 4
  store float %extract23, float addrspace(1)* %484, align 4
  store float %extract24, float addrspace(1)* %487, align 4
  store float %extract25, float addrspace(1)* %490, align 4
  store float %extract26, float addrspace(1)* %493, align 4
  store float %extract27, float addrspace(1)* %496, align 4
  store float %extract28, float addrspace(1)* %499, align 4
  store float %extract29, float addrspace(1)* %502, align 4
  store float %extract30, float addrspace(1)* %505, align 4
  store float %extract31, float addrspace(1)* %508, align 4
  %509 = add nsw <16 x i32> %vectorPHI, <i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320>
  %510 = and <16 x i32> %509, %vector
  %511 = extractelement <16 x i32> %510, i32 0
  %512 = sext i32 %511 to i64
  %513 = getelementptr inbounds float addrspace(1)* %output, i64 %512
  %514 = extractelement <16 x i32> %510, i32 1
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds float addrspace(1)* %output, i64 %515
  %517 = extractelement <16 x i32> %510, i32 2
  %518 = sext i32 %517 to i64
  %519 = getelementptr inbounds float addrspace(1)* %output, i64 %518
  %520 = extractelement <16 x i32> %510, i32 3
  %521 = sext i32 %520 to i64
  %522 = getelementptr inbounds float addrspace(1)* %output, i64 %521
  %523 = extractelement <16 x i32> %510, i32 4
  %524 = sext i32 %523 to i64
  %525 = getelementptr inbounds float addrspace(1)* %output, i64 %524
  %526 = extractelement <16 x i32> %510, i32 5
  %527 = sext i32 %526 to i64
  %528 = getelementptr inbounds float addrspace(1)* %output, i64 %527
  %529 = extractelement <16 x i32> %510, i32 6
  %530 = sext i32 %529 to i64
  %531 = getelementptr inbounds float addrspace(1)* %output, i64 %530
  %532 = extractelement <16 x i32> %510, i32 7
  %533 = sext i32 %532 to i64
  %534 = getelementptr inbounds float addrspace(1)* %output, i64 %533
  %535 = extractelement <16 x i32> %510, i32 8
  %536 = sext i32 %535 to i64
  %537 = getelementptr inbounds float addrspace(1)* %output, i64 %536
  %538 = extractelement <16 x i32> %510, i32 9
  %539 = sext i32 %538 to i64
  %540 = getelementptr inbounds float addrspace(1)* %output, i64 %539
  %541 = extractelement <16 x i32> %510, i32 10
  %542 = sext i32 %541 to i64
  %543 = getelementptr inbounds float addrspace(1)* %output, i64 %542
  %544 = extractelement <16 x i32> %510, i32 11
  %545 = sext i32 %544 to i64
  %546 = getelementptr inbounds float addrspace(1)* %output, i64 %545
  %547 = extractelement <16 x i32> %510, i32 12
  %548 = sext i32 %547 to i64
  %549 = getelementptr inbounds float addrspace(1)* %output, i64 %548
  %550 = extractelement <16 x i32> %510, i32 13
  %551 = sext i32 %550 to i64
  %552 = getelementptr inbounds float addrspace(1)* %output, i64 %551
  %553 = extractelement <16 x i32> %510, i32 14
  %554 = sext i32 %553 to i64
  %555 = getelementptr inbounds float addrspace(1)* %output, i64 %554
  %556 = extractelement <16 x i32> %510, i32 15
  %557 = sext i32 %556 to i64
  %558 = getelementptr inbounds float addrspace(1)* %output, i64 %557
  store float %extract16, float addrspace(1)* %513, align 4
  store float %extract17, float addrspace(1)* %516, align 4
  store float %extract18, float addrspace(1)* %519, align 4
  store float %extract19, float addrspace(1)* %522, align 4
  store float %extract20, float addrspace(1)* %525, align 4
  store float %extract21, float addrspace(1)* %528, align 4
  store float %extract22, float addrspace(1)* %531, align 4
  store float %extract23, float addrspace(1)* %534, align 4
  store float %extract24, float addrspace(1)* %537, align 4
  store float %extract25, float addrspace(1)* %540, align 4
  store float %extract26, float addrspace(1)* %543, align 4
  store float %extract27, float addrspace(1)* %546, align 4
  store float %extract28, float addrspace(1)* %549, align 4
  store float %extract29, float addrspace(1)* %552, align 4
  store float %extract30, float addrspace(1)* %555, align 4
  store float %extract31, float addrspace(1)* %558, align 4
  %559 = add nsw <16 x i32> %vectorPHI, <i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352>
  %560 = and <16 x i32> %559, %vector
  %561 = extractelement <16 x i32> %560, i32 0
  %562 = sext i32 %561 to i64
  %563 = getelementptr inbounds float addrspace(1)* %output, i64 %562
  %564 = extractelement <16 x i32> %560, i32 1
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds float addrspace(1)* %output, i64 %565
  %567 = extractelement <16 x i32> %560, i32 2
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds float addrspace(1)* %output, i64 %568
  %570 = extractelement <16 x i32> %560, i32 3
  %571 = sext i32 %570 to i64
  %572 = getelementptr inbounds float addrspace(1)* %output, i64 %571
  %573 = extractelement <16 x i32> %560, i32 4
  %574 = sext i32 %573 to i64
  %575 = getelementptr inbounds float addrspace(1)* %output, i64 %574
  %576 = extractelement <16 x i32> %560, i32 5
  %577 = sext i32 %576 to i64
  %578 = getelementptr inbounds float addrspace(1)* %output, i64 %577
  %579 = extractelement <16 x i32> %560, i32 6
  %580 = sext i32 %579 to i64
  %581 = getelementptr inbounds float addrspace(1)* %output, i64 %580
  %582 = extractelement <16 x i32> %560, i32 7
  %583 = sext i32 %582 to i64
  %584 = getelementptr inbounds float addrspace(1)* %output, i64 %583
  %585 = extractelement <16 x i32> %560, i32 8
  %586 = sext i32 %585 to i64
  %587 = getelementptr inbounds float addrspace(1)* %output, i64 %586
  %588 = extractelement <16 x i32> %560, i32 9
  %589 = sext i32 %588 to i64
  %590 = getelementptr inbounds float addrspace(1)* %output, i64 %589
  %591 = extractelement <16 x i32> %560, i32 10
  %592 = sext i32 %591 to i64
  %593 = getelementptr inbounds float addrspace(1)* %output, i64 %592
  %594 = extractelement <16 x i32> %560, i32 11
  %595 = sext i32 %594 to i64
  %596 = getelementptr inbounds float addrspace(1)* %output, i64 %595
  %597 = extractelement <16 x i32> %560, i32 12
  %598 = sext i32 %597 to i64
  %599 = getelementptr inbounds float addrspace(1)* %output, i64 %598
  %600 = extractelement <16 x i32> %560, i32 13
  %601 = sext i32 %600 to i64
  %602 = getelementptr inbounds float addrspace(1)* %output, i64 %601
  %603 = extractelement <16 x i32> %560, i32 14
  %604 = sext i32 %603 to i64
  %605 = getelementptr inbounds float addrspace(1)* %output, i64 %604
  %606 = extractelement <16 x i32> %560, i32 15
  %607 = sext i32 %606 to i64
  %608 = getelementptr inbounds float addrspace(1)* %output, i64 %607
  store float %extract16, float addrspace(1)* %563, align 4
  store float %extract17, float addrspace(1)* %566, align 4
  store float %extract18, float addrspace(1)* %569, align 4
  store float %extract19, float addrspace(1)* %572, align 4
  store float %extract20, float addrspace(1)* %575, align 4
  store float %extract21, float addrspace(1)* %578, align 4
  store float %extract22, float addrspace(1)* %581, align 4
  store float %extract23, float addrspace(1)* %584, align 4
  store float %extract24, float addrspace(1)* %587, align 4
  store float %extract25, float addrspace(1)* %590, align 4
  store float %extract26, float addrspace(1)* %593, align 4
  store float %extract27, float addrspace(1)* %596, align 4
  store float %extract28, float addrspace(1)* %599, align 4
  store float %extract29, float addrspace(1)* %602, align 4
  store float %extract30, float addrspace(1)* %605, align 4
  store float %extract31, float addrspace(1)* %608, align 4
  %609 = add nsw <16 x i32> %vectorPHI, <i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384>
  %610 = and <16 x i32> %609, %vector
  %611 = extractelement <16 x i32> %610, i32 0
  %612 = sext i32 %611 to i64
  %613 = getelementptr inbounds float addrspace(1)* %output, i64 %612
  %614 = extractelement <16 x i32> %610, i32 1
  %615 = sext i32 %614 to i64
  %616 = getelementptr inbounds float addrspace(1)* %output, i64 %615
  %617 = extractelement <16 x i32> %610, i32 2
  %618 = sext i32 %617 to i64
  %619 = getelementptr inbounds float addrspace(1)* %output, i64 %618
  %620 = extractelement <16 x i32> %610, i32 3
  %621 = sext i32 %620 to i64
  %622 = getelementptr inbounds float addrspace(1)* %output, i64 %621
  %623 = extractelement <16 x i32> %610, i32 4
  %624 = sext i32 %623 to i64
  %625 = getelementptr inbounds float addrspace(1)* %output, i64 %624
  %626 = extractelement <16 x i32> %610, i32 5
  %627 = sext i32 %626 to i64
  %628 = getelementptr inbounds float addrspace(1)* %output, i64 %627
  %629 = extractelement <16 x i32> %610, i32 6
  %630 = sext i32 %629 to i64
  %631 = getelementptr inbounds float addrspace(1)* %output, i64 %630
  %632 = extractelement <16 x i32> %610, i32 7
  %633 = sext i32 %632 to i64
  %634 = getelementptr inbounds float addrspace(1)* %output, i64 %633
  %635 = extractelement <16 x i32> %610, i32 8
  %636 = sext i32 %635 to i64
  %637 = getelementptr inbounds float addrspace(1)* %output, i64 %636
  %638 = extractelement <16 x i32> %610, i32 9
  %639 = sext i32 %638 to i64
  %640 = getelementptr inbounds float addrspace(1)* %output, i64 %639
  %641 = extractelement <16 x i32> %610, i32 10
  %642 = sext i32 %641 to i64
  %643 = getelementptr inbounds float addrspace(1)* %output, i64 %642
  %644 = extractelement <16 x i32> %610, i32 11
  %645 = sext i32 %644 to i64
  %646 = getelementptr inbounds float addrspace(1)* %output, i64 %645
  %647 = extractelement <16 x i32> %610, i32 12
  %648 = sext i32 %647 to i64
  %649 = getelementptr inbounds float addrspace(1)* %output, i64 %648
  %650 = extractelement <16 x i32> %610, i32 13
  %651 = sext i32 %650 to i64
  %652 = getelementptr inbounds float addrspace(1)* %output, i64 %651
  %653 = extractelement <16 x i32> %610, i32 14
  %654 = sext i32 %653 to i64
  %655 = getelementptr inbounds float addrspace(1)* %output, i64 %654
  %656 = extractelement <16 x i32> %610, i32 15
  %657 = sext i32 %656 to i64
  %658 = getelementptr inbounds float addrspace(1)* %output, i64 %657
  store float %extract16, float addrspace(1)* %613, align 4
  store float %extract17, float addrspace(1)* %616, align 4
  store float %extract18, float addrspace(1)* %619, align 4
  store float %extract19, float addrspace(1)* %622, align 4
  store float %extract20, float addrspace(1)* %625, align 4
  store float %extract21, float addrspace(1)* %628, align 4
  store float %extract22, float addrspace(1)* %631, align 4
  store float %extract23, float addrspace(1)* %634, align 4
  store float %extract24, float addrspace(1)* %637, align 4
  store float %extract25, float addrspace(1)* %640, align 4
  store float %extract26, float addrspace(1)* %643, align 4
  store float %extract27, float addrspace(1)* %646, align 4
  store float %extract28, float addrspace(1)* %649, align 4
  store float %extract29, float addrspace(1)* %652, align 4
  store float %extract30, float addrspace(1)* %655, align 4
  store float %extract31, float addrspace(1)* %658, align 4
  %659 = add nsw <16 x i32> %vectorPHI, <i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416>
  %660 = and <16 x i32> %659, %vector
  %661 = extractelement <16 x i32> %660, i32 0
  %662 = sext i32 %661 to i64
  %663 = getelementptr inbounds float addrspace(1)* %output, i64 %662
  %664 = extractelement <16 x i32> %660, i32 1
  %665 = sext i32 %664 to i64
  %666 = getelementptr inbounds float addrspace(1)* %output, i64 %665
  %667 = extractelement <16 x i32> %660, i32 2
  %668 = sext i32 %667 to i64
  %669 = getelementptr inbounds float addrspace(1)* %output, i64 %668
  %670 = extractelement <16 x i32> %660, i32 3
  %671 = sext i32 %670 to i64
  %672 = getelementptr inbounds float addrspace(1)* %output, i64 %671
  %673 = extractelement <16 x i32> %660, i32 4
  %674 = sext i32 %673 to i64
  %675 = getelementptr inbounds float addrspace(1)* %output, i64 %674
  %676 = extractelement <16 x i32> %660, i32 5
  %677 = sext i32 %676 to i64
  %678 = getelementptr inbounds float addrspace(1)* %output, i64 %677
  %679 = extractelement <16 x i32> %660, i32 6
  %680 = sext i32 %679 to i64
  %681 = getelementptr inbounds float addrspace(1)* %output, i64 %680
  %682 = extractelement <16 x i32> %660, i32 7
  %683 = sext i32 %682 to i64
  %684 = getelementptr inbounds float addrspace(1)* %output, i64 %683
  %685 = extractelement <16 x i32> %660, i32 8
  %686 = sext i32 %685 to i64
  %687 = getelementptr inbounds float addrspace(1)* %output, i64 %686
  %688 = extractelement <16 x i32> %660, i32 9
  %689 = sext i32 %688 to i64
  %690 = getelementptr inbounds float addrspace(1)* %output, i64 %689
  %691 = extractelement <16 x i32> %660, i32 10
  %692 = sext i32 %691 to i64
  %693 = getelementptr inbounds float addrspace(1)* %output, i64 %692
  %694 = extractelement <16 x i32> %660, i32 11
  %695 = sext i32 %694 to i64
  %696 = getelementptr inbounds float addrspace(1)* %output, i64 %695
  %697 = extractelement <16 x i32> %660, i32 12
  %698 = sext i32 %697 to i64
  %699 = getelementptr inbounds float addrspace(1)* %output, i64 %698
  %700 = extractelement <16 x i32> %660, i32 13
  %701 = sext i32 %700 to i64
  %702 = getelementptr inbounds float addrspace(1)* %output, i64 %701
  %703 = extractelement <16 x i32> %660, i32 14
  %704 = sext i32 %703 to i64
  %705 = getelementptr inbounds float addrspace(1)* %output, i64 %704
  %706 = extractelement <16 x i32> %660, i32 15
  %707 = sext i32 %706 to i64
  %708 = getelementptr inbounds float addrspace(1)* %output, i64 %707
  store float %extract16, float addrspace(1)* %663, align 4
  store float %extract17, float addrspace(1)* %666, align 4
  store float %extract18, float addrspace(1)* %669, align 4
  store float %extract19, float addrspace(1)* %672, align 4
  store float %extract20, float addrspace(1)* %675, align 4
  store float %extract21, float addrspace(1)* %678, align 4
  store float %extract22, float addrspace(1)* %681, align 4
  store float %extract23, float addrspace(1)* %684, align 4
  store float %extract24, float addrspace(1)* %687, align 4
  store float %extract25, float addrspace(1)* %690, align 4
  store float %extract26, float addrspace(1)* %693, align 4
  store float %extract27, float addrspace(1)* %696, align 4
  store float %extract28, float addrspace(1)* %699, align 4
  store float %extract29, float addrspace(1)* %702, align 4
  store float %extract30, float addrspace(1)* %705, align 4
  store float %extract31, float addrspace(1)* %708, align 4
  %709 = add nsw <16 x i32> %vectorPHI, <i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448>
  %710 = and <16 x i32> %709, %vector
  %711 = extractelement <16 x i32> %710, i32 0
  %712 = sext i32 %711 to i64
  %713 = getelementptr inbounds float addrspace(1)* %output, i64 %712
  %714 = extractelement <16 x i32> %710, i32 1
  %715 = sext i32 %714 to i64
  %716 = getelementptr inbounds float addrspace(1)* %output, i64 %715
  %717 = extractelement <16 x i32> %710, i32 2
  %718 = sext i32 %717 to i64
  %719 = getelementptr inbounds float addrspace(1)* %output, i64 %718
  %720 = extractelement <16 x i32> %710, i32 3
  %721 = sext i32 %720 to i64
  %722 = getelementptr inbounds float addrspace(1)* %output, i64 %721
  %723 = extractelement <16 x i32> %710, i32 4
  %724 = sext i32 %723 to i64
  %725 = getelementptr inbounds float addrspace(1)* %output, i64 %724
  %726 = extractelement <16 x i32> %710, i32 5
  %727 = sext i32 %726 to i64
  %728 = getelementptr inbounds float addrspace(1)* %output, i64 %727
  %729 = extractelement <16 x i32> %710, i32 6
  %730 = sext i32 %729 to i64
  %731 = getelementptr inbounds float addrspace(1)* %output, i64 %730
  %732 = extractelement <16 x i32> %710, i32 7
  %733 = sext i32 %732 to i64
  %734 = getelementptr inbounds float addrspace(1)* %output, i64 %733
  %735 = extractelement <16 x i32> %710, i32 8
  %736 = sext i32 %735 to i64
  %737 = getelementptr inbounds float addrspace(1)* %output, i64 %736
  %738 = extractelement <16 x i32> %710, i32 9
  %739 = sext i32 %738 to i64
  %740 = getelementptr inbounds float addrspace(1)* %output, i64 %739
  %741 = extractelement <16 x i32> %710, i32 10
  %742 = sext i32 %741 to i64
  %743 = getelementptr inbounds float addrspace(1)* %output, i64 %742
  %744 = extractelement <16 x i32> %710, i32 11
  %745 = sext i32 %744 to i64
  %746 = getelementptr inbounds float addrspace(1)* %output, i64 %745
  %747 = extractelement <16 x i32> %710, i32 12
  %748 = sext i32 %747 to i64
  %749 = getelementptr inbounds float addrspace(1)* %output, i64 %748
  %750 = extractelement <16 x i32> %710, i32 13
  %751 = sext i32 %750 to i64
  %752 = getelementptr inbounds float addrspace(1)* %output, i64 %751
  %753 = extractelement <16 x i32> %710, i32 14
  %754 = sext i32 %753 to i64
  %755 = getelementptr inbounds float addrspace(1)* %output, i64 %754
  %756 = extractelement <16 x i32> %710, i32 15
  %757 = sext i32 %756 to i64
  %758 = getelementptr inbounds float addrspace(1)* %output, i64 %757
  store float %extract16, float addrspace(1)* %713, align 4
  store float %extract17, float addrspace(1)* %716, align 4
  store float %extract18, float addrspace(1)* %719, align 4
  store float %extract19, float addrspace(1)* %722, align 4
  store float %extract20, float addrspace(1)* %725, align 4
  store float %extract21, float addrspace(1)* %728, align 4
  store float %extract22, float addrspace(1)* %731, align 4
  store float %extract23, float addrspace(1)* %734, align 4
  store float %extract24, float addrspace(1)* %737, align 4
  store float %extract25, float addrspace(1)* %740, align 4
  store float %extract26, float addrspace(1)* %743, align 4
  store float %extract27, float addrspace(1)* %746, align 4
  store float %extract28, float addrspace(1)* %749, align 4
  store float %extract29, float addrspace(1)* %752, align 4
  store float %extract30, float addrspace(1)* %755, align 4
  store float %extract31, float addrspace(1)* %758, align 4
  %759 = add nsw <16 x i32> %vectorPHI, <i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480>
  %760 = and <16 x i32> %759, %vector
  %761 = extractelement <16 x i32> %760, i32 0
  %762 = sext i32 %761 to i64
  %763 = getelementptr inbounds float addrspace(1)* %output, i64 %762
  %764 = extractelement <16 x i32> %760, i32 1
  %765 = sext i32 %764 to i64
  %766 = getelementptr inbounds float addrspace(1)* %output, i64 %765
  %767 = extractelement <16 x i32> %760, i32 2
  %768 = sext i32 %767 to i64
  %769 = getelementptr inbounds float addrspace(1)* %output, i64 %768
  %770 = extractelement <16 x i32> %760, i32 3
  %771 = sext i32 %770 to i64
  %772 = getelementptr inbounds float addrspace(1)* %output, i64 %771
  %773 = extractelement <16 x i32> %760, i32 4
  %774 = sext i32 %773 to i64
  %775 = getelementptr inbounds float addrspace(1)* %output, i64 %774
  %776 = extractelement <16 x i32> %760, i32 5
  %777 = sext i32 %776 to i64
  %778 = getelementptr inbounds float addrspace(1)* %output, i64 %777
  %779 = extractelement <16 x i32> %760, i32 6
  %780 = sext i32 %779 to i64
  %781 = getelementptr inbounds float addrspace(1)* %output, i64 %780
  %782 = extractelement <16 x i32> %760, i32 7
  %783 = sext i32 %782 to i64
  %784 = getelementptr inbounds float addrspace(1)* %output, i64 %783
  %785 = extractelement <16 x i32> %760, i32 8
  %786 = sext i32 %785 to i64
  %787 = getelementptr inbounds float addrspace(1)* %output, i64 %786
  %788 = extractelement <16 x i32> %760, i32 9
  %789 = sext i32 %788 to i64
  %790 = getelementptr inbounds float addrspace(1)* %output, i64 %789
  %791 = extractelement <16 x i32> %760, i32 10
  %792 = sext i32 %791 to i64
  %793 = getelementptr inbounds float addrspace(1)* %output, i64 %792
  %794 = extractelement <16 x i32> %760, i32 11
  %795 = sext i32 %794 to i64
  %796 = getelementptr inbounds float addrspace(1)* %output, i64 %795
  %797 = extractelement <16 x i32> %760, i32 12
  %798 = sext i32 %797 to i64
  %799 = getelementptr inbounds float addrspace(1)* %output, i64 %798
  %800 = extractelement <16 x i32> %760, i32 13
  %801 = sext i32 %800 to i64
  %802 = getelementptr inbounds float addrspace(1)* %output, i64 %801
  %803 = extractelement <16 x i32> %760, i32 14
  %804 = sext i32 %803 to i64
  %805 = getelementptr inbounds float addrspace(1)* %output, i64 %804
  %806 = extractelement <16 x i32> %760, i32 15
  %807 = sext i32 %806 to i64
  %808 = getelementptr inbounds float addrspace(1)* %output, i64 %807
  store float %extract16, float addrspace(1)* %763, align 4
  store float %extract17, float addrspace(1)* %766, align 4
  store float %extract18, float addrspace(1)* %769, align 4
  store float %extract19, float addrspace(1)* %772, align 4
  store float %extract20, float addrspace(1)* %775, align 4
  store float %extract21, float addrspace(1)* %778, align 4
  store float %extract22, float addrspace(1)* %781, align 4
  store float %extract23, float addrspace(1)* %784, align 4
  store float %extract24, float addrspace(1)* %787, align 4
  store float %extract25, float addrspace(1)* %790, align 4
  store float %extract26, float addrspace(1)* %793, align 4
  store float %extract27, float addrspace(1)* %796, align 4
  store float %extract28, float addrspace(1)* %799, align 4
  store float %extract29, float addrspace(1)* %802, align 4
  store float %extract30, float addrspace(1)* %805, align 4
  store float %extract31, float addrspace(1)* %808, align 4
  %809 = add nsw i32 %j.01, 1
  %810 = add nsw <16 x i32> %vectorPHI, <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>
  %811 = and <16 x i32> %810, %vector
  %exitcond = icmp eq i32 %809, 1024
  br i1 %exitcond, label %._crit_edge, label %9

._crit_edge:                                      ; preds = %9
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB272

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB272:                                        ; preds = %._crit_edge
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
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %15 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = add i64 %16, %18
  %20 = trunc i64 %19 to i32
  %21 = sitofp i32 %20 to float
  br label %22

; <label>:22                                      ; preds = %22, %SyncBB.i
  %s.02.i = phi i32 [ %20, %SyncBB.i ], [ %88, %22 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %86, %22 ]
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
  br label %SyncBB.i

__writeGlobalMemoryCoalesced_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.writeGlobalMemoryCoalesced(i8* %pBuffer) {
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
  %22 = sitofp <16 x i32> %21 to <16 x float>
  %extract16.i = extractelement <16 x float> %22, i32 0
  %extract17.i = extractelement <16 x float> %22, i32 1
  %extract18.i = extractelement <16 x float> %22, i32 2
  %extract19.i = extractelement <16 x float> %22, i32 3
  %extract20.i = extractelement <16 x float> %22, i32 4
  %extract21.i = extractelement <16 x float> %22, i32 5
  %extract22.i = extractelement <16 x float> %22, i32 6
  %extract23.i = extractelement <16 x float> %22, i32 7
  %extract24.i = extractelement <16 x float> %22, i32 8
  %extract25.i = extractelement <16 x float> %22, i32 9
  %extract26.i = extractelement <16 x float> %22, i32 10
  %extract27.i = extractelement <16 x float> %22, i32 11
  %extract28.i = extractelement <16 x float> %22, i32 12
  %extract29.i = extractelement <16 x float> %22, i32 13
  %extract30.i = extractelement <16 x float> %22, i32 14
  %extract31.i = extractelement <16 x float> %22, i32 15
  br label %23

; <label>:23                                      ; preds = %23, %SyncBB.i
  %vectorPHI.i = phi <16 x i32> [ %21, %SyncBB.i ], [ %825, %23 ]
  %j.01.i = phi i32 [ 0, %SyncBB.i ], [ %823, %23 ]
  %24 = and <16 x i32> %vectorPHI.i, %vector.i
  %25 = extractelement <16 x i32> %24, i32 0
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds float addrspace(1)* %1, i64 %26
  %28 = extractelement <16 x i32> %24, i32 1
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds float addrspace(1)* %1, i64 %29
  %31 = extractelement <16 x i32> %24, i32 2
  %32 = sext i32 %31 to i64
  %33 = getelementptr inbounds float addrspace(1)* %1, i64 %32
  %34 = extractelement <16 x i32> %24, i32 3
  %35 = sext i32 %34 to i64
  %36 = getelementptr inbounds float addrspace(1)* %1, i64 %35
  %37 = extractelement <16 x i32> %24, i32 4
  %38 = sext i32 %37 to i64
  %39 = getelementptr inbounds float addrspace(1)* %1, i64 %38
  %40 = extractelement <16 x i32> %24, i32 5
  %41 = sext i32 %40 to i64
  %42 = getelementptr inbounds float addrspace(1)* %1, i64 %41
  %43 = extractelement <16 x i32> %24, i32 6
  %44 = sext i32 %43 to i64
  %45 = getelementptr inbounds float addrspace(1)* %1, i64 %44
  %46 = extractelement <16 x i32> %24, i32 7
  %47 = sext i32 %46 to i64
  %48 = getelementptr inbounds float addrspace(1)* %1, i64 %47
  %49 = extractelement <16 x i32> %24, i32 8
  %50 = sext i32 %49 to i64
  %51 = getelementptr inbounds float addrspace(1)* %1, i64 %50
  %52 = extractelement <16 x i32> %24, i32 9
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds float addrspace(1)* %1, i64 %53
  %55 = extractelement <16 x i32> %24, i32 10
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %56
  %58 = extractelement <16 x i32> %24, i32 11
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds float addrspace(1)* %1, i64 %59
  %61 = extractelement <16 x i32> %24, i32 12
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %62
  %64 = extractelement <16 x i32> %24, i32 13
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %65
  %67 = extractelement <16 x i32> %24, i32 14
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds float addrspace(1)* %1, i64 %68
  %70 = extractelement <16 x i32> %24, i32 15
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %1, i64 %71
  store float %extract16.i, float addrspace(1)* %27, align 4
  store float %extract17.i, float addrspace(1)* %30, align 4
  store float %extract18.i, float addrspace(1)* %33, align 4
  store float %extract19.i, float addrspace(1)* %36, align 4
  store float %extract20.i, float addrspace(1)* %39, align 4
  store float %extract21.i, float addrspace(1)* %42, align 4
  store float %extract22.i, float addrspace(1)* %45, align 4
  store float %extract23.i, float addrspace(1)* %48, align 4
  store float %extract24.i, float addrspace(1)* %51, align 4
  store float %extract25.i, float addrspace(1)* %54, align 4
  store float %extract26.i, float addrspace(1)* %57, align 4
  store float %extract27.i, float addrspace(1)* %60, align 4
  store float %extract28.i, float addrspace(1)* %63, align 4
  store float %extract29.i, float addrspace(1)* %66, align 4
  store float %extract30.i, float addrspace(1)* %69, align 4
  store float %extract31.i, float addrspace(1)* %72, align 4
  %73 = add nsw <16 x i32> %vectorPHI.i, <i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32>
  %74 = and <16 x i32> %73, %vector.i
  %75 = extractelement <16 x i32> %74, i32 0
  %76 = sext i32 %75 to i64
  %77 = getelementptr inbounds float addrspace(1)* %1, i64 %76
  %78 = extractelement <16 x i32> %74, i32 1
  %79 = sext i32 %78 to i64
  %80 = getelementptr inbounds float addrspace(1)* %1, i64 %79
  %81 = extractelement <16 x i32> %74, i32 2
  %82 = sext i32 %81 to i64
  %83 = getelementptr inbounds float addrspace(1)* %1, i64 %82
  %84 = extractelement <16 x i32> %74, i32 3
  %85 = sext i32 %84 to i64
  %86 = getelementptr inbounds float addrspace(1)* %1, i64 %85
  %87 = extractelement <16 x i32> %74, i32 4
  %88 = sext i32 %87 to i64
  %89 = getelementptr inbounds float addrspace(1)* %1, i64 %88
  %90 = extractelement <16 x i32> %74, i32 5
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds float addrspace(1)* %1, i64 %91
  %93 = extractelement <16 x i32> %74, i32 6
  %94 = sext i32 %93 to i64
  %95 = getelementptr inbounds float addrspace(1)* %1, i64 %94
  %96 = extractelement <16 x i32> %74, i32 7
  %97 = sext i32 %96 to i64
  %98 = getelementptr inbounds float addrspace(1)* %1, i64 %97
  %99 = extractelement <16 x i32> %74, i32 8
  %100 = sext i32 %99 to i64
  %101 = getelementptr inbounds float addrspace(1)* %1, i64 %100
  %102 = extractelement <16 x i32> %74, i32 9
  %103 = sext i32 %102 to i64
  %104 = getelementptr inbounds float addrspace(1)* %1, i64 %103
  %105 = extractelement <16 x i32> %74, i32 10
  %106 = sext i32 %105 to i64
  %107 = getelementptr inbounds float addrspace(1)* %1, i64 %106
  %108 = extractelement <16 x i32> %74, i32 11
  %109 = sext i32 %108 to i64
  %110 = getelementptr inbounds float addrspace(1)* %1, i64 %109
  %111 = extractelement <16 x i32> %74, i32 12
  %112 = sext i32 %111 to i64
  %113 = getelementptr inbounds float addrspace(1)* %1, i64 %112
  %114 = extractelement <16 x i32> %74, i32 13
  %115 = sext i32 %114 to i64
  %116 = getelementptr inbounds float addrspace(1)* %1, i64 %115
  %117 = extractelement <16 x i32> %74, i32 14
  %118 = sext i32 %117 to i64
  %119 = getelementptr inbounds float addrspace(1)* %1, i64 %118
  %120 = extractelement <16 x i32> %74, i32 15
  %121 = sext i32 %120 to i64
  %122 = getelementptr inbounds float addrspace(1)* %1, i64 %121
  store float %extract16.i, float addrspace(1)* %77, align 4
  store float %extract17.i, float addrspace(1)* %80, align 4
  store float %extract18.i, float addrspace(1)* %83, align 4
  store float %extract19.i, float addrspace(1)* %86, align 4
  store float %extract20.i, float addrspace(1)* %89, align 4
  store float %extract21.i, float addrspace(1)* %92, align 4
  store float %extract22.i, float addrspace(1)* %95, align 4
  store float %extract23.i, float addrspace(1)* %98, align 4
  store float %extract24.i, float addrspace(1)* %101, align 4
  store float %extract25.i, float addrspace(1)* %104, align 4
  store float %extract26.i, float addrspace(1)* %107, align 4
  store float %extract27.i, float addrspace(1)* %110, align 4
  store float %extract28.i, float addrspace(1)* %113, align 4
  store float %extract29.i, float addrspace(1)* %116, align 4
  store float %extract30.i, float addrspace(1)* %119, align 4
  store float %extract31.i, float addrspace(1)* %122, align 4
  %123 = add nsw <16 x i32> %vectorPHI.i, <i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64, i32 64>
  %124 = and <16 x i32> %123, %vector.i
  %125 = extractelement <16 x i32> %124, i32 0
  %126 = sext i32 %125 to i64
  %127 = getelementptr inbounds float addrspace(1)* %1, i64 %126
  %128 = extractelement <16 x i32> %124, i32 1
  %129 = sext i32 %128 to i64
  %130 = getelementptr inbounds float addrspace(1)* %1, i64 %129
  %131 = extractelement <16 x i32> %124, i32 2
  %132 = sext i32 %131 to i64
  %133 = getelementptr inbounds float addrspace(1)* %1, i64 %132
  %134 = extractelement <16 x i32> %124, i32 3
  %135 = sext i32 %134 to i64
  %136 = getelementptr inbounds float addrspace(1)* %1, i64 %135
  %137 = extractelement <16 x i32> %124, i32 4
  %138 = sext i32 %137 to i64
  %139 = getelementptr inbounds float addrspace(1)* %1, i64 %138
  %140 = extractelement <16 x i32> %124, i32 5
  %141 = sext i32 %140 to i64
  %142 = getelementptr inbounds float addrspace(1)* %1, i64 %141
  %143 = extractelement <16 x i32> %124, i32 6
  %144 = sext i32 %143 to i64
  %145 = getelementptr inbounds float addrspace(1)* %1, i64 %144
  %146 = extractelement <16 x i32> %124, i32 7
  %147 = sext i32 %146 to i64
  %148 = getelementptr inbounds float addrspace(1)* %1, i64 %147
  %149 = extractelement <16 x i32> %124, i32 8
  %150 = sext i32 %149 to i64
  %151 = getelementptr inbounds float addrspace(1)* %1, i64 %150
  %152 = extractelement <16 x i32> %124, i32 9
  %153 = sext i32 %152 to i64
  %154 = getelementptr inbounds float addrspace(1)* %1, i64 %153
  %155 = extractelement <16 x i32> %124, i32 10
  %156 = sext i32 %155 to i64
  %157 = getelementptr inbounds float addrspace(1)* %1, i64 %156
  %158 = extractelement <16 x i32> %124, i32 11
  %159 = sext i32 %158 to i64
  %160 = getelementptr inbounds float addrspace(1)* %1, i64 %159
  %161 = extractelement <16 x i32> %124, i32 12
  %162 = sext i32 %161 to i64
  %163 = getelementptr inbounds float addrspace(1)* %1, i64 %162
  %164 = extractelement <16 x i32> %124, i32 13
  %165 = sext i32 %164 to i64
  %166 = getelementptr inbounds float addrspace(1)* %1, i64 %165
  %167 = extractelement <16 x i32> %124, i32 14
  %168 = sext i32 %167 to i64
  %169 = getelementptr inbounds float addrspace(1)* %1, i64 %168
  %170 = extractelement <16 x i32> %124, i32 15
  %171 = sext i32 %170 to i64
  %172 = getelementptr inbounds float addrspace(1)* %1, i64 %171
  store float %extract16.i, float addrspace(1)* %127, align 4
  store float %extract17.i, float addrspace(1)* %130, align 4
  store float %extract18.i, float addrspace(1)* %133, align 4
  store float %extract19.i, float addrspace(1)* %136, align 4
  store float %extract20.i, float addrspace(1)* %139, align 4
  store float %extract21.i, float addrspace(1)* %142, align 4
  store float %extract22.i, float addrspace(1)* %145, align 4
  store float %extract23.i, float addrspace(1)* %148, align 4
  store float %extract24.i, float addrspace(1)* %151, align 4
  store float %extract25.i, float addrspace(1)* %154, align 4
  store float %extract26.i, float addrspace(1)* %157, align 4
  store float %extract27.i, float addrspace(1)* %160, align 4
  store float %extract28.i, float addrspace(1)* %163, align 4
  store float %extract29.i, float addrspace(1)* %166, align 4
  store float %extract30.i, float addrspace(1)* %169, align 4
  store float %extract31.i, float addrspace(1)* %172, align 4
  %173 = add nsw <16 x i32> %vectorPHI.i, <i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96, i32 96>
  %174 = and <16 x i32> %173, %vector.i
  %175 = extractelement <16 x i32> %174, i32 0
  %176 = sext i32 %175 to i64
  %177 = getelementptr inbounds float addrspace(1)* %1, i64 %176
  %178 = extractelement <16 x i32> %174, i32 1
  %179 = sext i32 %178 to i64
  %180 = getelementptr inbounds float addrspace(1)* %1, i64 %179
  %181 = extractelement <16 x i32> %174, i32 2
  %182 = sext i32 %181 to i64
  %183 = getelementptr inbounds float addrspace(1)* %1, i64 %182
  %184 = extractelement <16 x i32> %174, i32 3
  %185 = sext i32 %184 to i64
  %186 = getelementptr inbounds float addrspace(1)* %1, i64 %185
  %187 = extractelement <16 x i32> %174, i32 4
  %188 = sext i32 %187 to i64
  %189 = getelementptr inbounds float addrspace(1)* %1, i64 %188
  %190 = extractelement <16 x i32> %174, i32 5
  %191 = sext i32 %190 to i64
  %192 = getelementptr inbounds float addrspace(1)* %1, i64 %191
  %193 = extractelement <16 x i32> %174, i32 6
  %194 = sext i32 %193 to i64
  %195 = getelementptr inbounds float addrspace(1)* %1, i64 %194
  %196 = extractelement <16 x i32> %174, i32 7
  %197 = sext i32 %196 to i64
  %198 = getelementptr inbounds float addrspace(1)* %1, i64 %197
  %199 = extractelement <16 x i32> %174, i32 8
  %200 = sext i32 %199 to i64
  %201 = getelementptr inbounds float addrspace(1)* %1, i64 %200
  %202 = extractelement <16 x i32> %174, i32 9
  %203 = sext i32 %202 to i64
  %204 = getelementptr inbounds float addrspace(1)* %1, i64 %203
  %205 = extractelement <16 x i32> %174, i32 10
  %206 = sext i32 %205 to i64
  %207 = getelementptr inbounds float addrspace(1)* %1, i64 %206
  %208 = extractelement <16 x i32> %174, i32 11
  %209 = sext i32 %208 to i64
  %210 = getelementptr inbounds float addrspace(1)* %1, i64 %209
  %211 = extractelement <16 x i32> %174, i32 12
  %212 = sext i32 %211 to i64
  %213 = getelementptr inbounds float addrspace(1)* %1, i64 %212
  %214 = extractelement <16 x i32> %174, i32 13
  %215 = sext i32 %214 to i64
  %216 = getelementptr inbounds float addrspace(1)* %1, i64 %215
  %217 = extractelement <16 x i32> %174, i32 14
  %218 = sext i32 %217 to i64
  %219 = getelementptr inbounds float addrspace(1)* %1, i64 %218
  %220 = extractelement <16 x i32> %174, i32 15
  %221 = sext i32 %220 to i64
  %222 = getelementptr inbounds float addrspace(1)* %1, i64 %221
  store float %extract16.i, float addrspace(1)* %177, align 4
  store float %extract17.i, float addrspace(1)* %180, align 4
  store float %extract18.i, float addrspace(1)* %183, align 4
  store float %extract19.i, float addrspace(1)* %186, align 4
  store float %extract20.i, float addrspace(1)* %189, align 4
  store float %extract21.i, float addrspace(1)* %192, align 4
  store float %extract22.i, float addrspace(1)* %195, align 4
  store float %extract23.i, float addrspace(1)* %198, align 4
  store float %extract24.i, float addrspace(1)* %201, align 4
  store float %extract25.i, float addrspace(1)* %204, align 4
  store float %extract26.i, float addrspace(1)* %207, align 4
  store float %extract27.i, float addrspace(1)* %210, align 4
  store float %extract28.i, float addrspace(1)* %213, align 4
  store float %extract29.i, float addrspace(1)* %216, align 4
  store float %extract30.i, float addrspace(1)* %219, align 4
  store float %extract31.i, float addrspace(1)* %222, align 4
  %223 = add nsw <16 x i32> %vectorPHI.i, <i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128, i32 128>
  %224 = and <16 x i32> %223, %vector.i
  %225 = extractelement <16 x i32> %224, i32 0
  %226 = sext i32 %225 to i64
  %227 = getelementptr inbounds float addrspace(1)* %1, i64 %226
  %228 = extractelement <16 x i32> %224, i32 1
  %229 = sext i32 %228 to i64
  %230 = getelementptr inbounds float addrspace(1)* %1, i64 %229
  %231 = extractelement <16 x i32> %224, i32 2
  %232 = sext i32 %231 to i64
  %233 = getelementptr inbounds float addrspace(1)* %1, i64 %232
  %234 = extractelement <16 x i32> %224, i32 3
  %235 = sext i32 %234 to i64
  %236 = getelementptr inbounds float addrspace(1)* %1, i64 %235
  %237 = extractelement <16 x i32> %224, i32 4
  %238 = sext i32 %237 to i64
  %239 = getelementptr inbounds float addrspace(1)* %1, i64 %238
  %240 = extractelement <16 x i32> %224, i32 5
  %241 = sext i32 %240 to i64
  %242 = getelementptr inbounds float addrspace(1)* %1, i64 %241
  %243 = extractelement <16 x i32> %224, i32 6
  %244 = sext i32 %243 to i64
  %245 = getelementptr inbounds float addrspace(1)* %1, i64 %244
  %246 = extractelement <16 x i32> %224, i32 7
  %247 = sext i32 %246 to i64
  %248 = getelementptr inbounds float addrspace(1)* %1, i64 %247
  %249 = extractelement <16 x i32> %224, i32 8
  %250 = sext i32 %249 to i64
  %251 = getelementptr inbounds float addrspace(1)* %1, i64 %250
  %252 = extractelement <16 x i32> %224, i32 9
  %253 = sext i32 %252 to i64
  %254 = getelementptr inbounds float addrspace(1)* %1, i64 %253
  %255 = extractelement <16 x i32> %224, i32 10
  %256 = sext i32 %255 to i64
  %257 = getelementptr inbounds float addrspace(1)* %1, i64 %256
  %258 = extractelement <16 x i32> %224, i32 11
  %259 = sext i32 %258 to i64
  %260 = getelementptr inbounds float addrspace(1)* %1, i64 %259
  %261 = extractelement <16 x i32> %224, i32 12
  %262 = sext i32 %261 to i64
  %263 = getelementptr inbounds float addrspace(1)* %1, i64 %262
  %264 = extractelement <16 x i32> %224, i32 13
  %265 = sext i32 %264 to i64
  %266 = getelementptr inbounds float addrspace(1)* %1, i64 %265
  %267 = extractelement <16 x i32> %224, i32 14
  %268 = sext i32 %267 to i64
  %269 = getelementptr inbounds float addrspace(1)* %1, i64 %268
  %270 = extractelement <16 x i32> %224, i32 15
  %271 = sext i32 %270 to i64
  %272 = getelementptr inbounds float addrspace(1)* %1, i64 %271
  store float %extract16.i, float addrspace(1)* %227, align 4
  store float %extract17.i, float addrspace(1)* %230, align 4
  store float %extract18.i, float addrspace(1)* %233, align 4
  store float %extract19.i, float addrspace(1)* %236, align 4
  store float %extract20.i, float addrspace(1)* %239, align 4
  store float %extract21.i, float addrspace(1)* %242, align 4
  store float %extract22.i, float addrspace(1)* %245, align 4
  store float %extract23.i, float addrspace(1)* %248, align 4
  store float %extract24.i, float addrspace(1)* %251, align 4
  store float %extract25.i, float addrspace(1)* %254, align 4
  store float %extract26.i, float addrspace(1)* %257, align 4
  store float %extract27.i, float addrspace(1)* %260, align 4
  store float %extract28.i, float addrspace(1)* %263, align 4
  store float %extract29.i, float addrspace(1)* %266, align 4
  store float %extract30.i, float addrspace(1)* %269, align 4
  store float %extract31.i, float addrspace(1)* %272, align 4
  %273 = add nsw <16 x i32> %vectorPHI.i, <i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160, i32 160>
  %274 = and <16 x i32> %273, %vector.i
  %275 = extractelement <16 x i32> %274, i32 0
  %276 = sext i32 %275 to i64
  %277 = getelementptr inbounds float addrspace(1)* %1, i64 %276
  %278 = extractelement <16 x i32> %274, i32 1
  %279 = sext i32 %278 to i64
  %280 = getelementptr inbounds float addrspace(1)* %1, i64 %279
  %281 = extractelement <16 x i32> %274, i32 2
  %282 = sext i32 %281 to i64
  %283 = getelementptr inbounds float addrspace(1)* %1, i64 %282
  %284 = extractelement <16 x i32> %274, i32 3
  %285 = sext i32 %284 to i64
  %286 = getelementptr inbounds float addrspace(1)* %1, i64 %285
  %287 = extractelement <16 x i32> %274, i32 4
  %288 = sext i32 %287 to i64
  %289 = getelementptr inbounds float addrspace(1)* %1, i64 %288
  %290 = extractelement <16 x i32> %274, i32 5
  %291 = sext i32 %290 to i64
  %292 = getelementptr inbounds float addrspace(1)* %1, i64 %291
  %293 = extractelement <16 x i32> %274, i32 6
  %294 = sext i32 %293 to i64
  %295 = getelementptr inbounds float addrspace(1)* %1, i64 %294
  %296 = extractelement <16 x i32> %274, i32 7
  %297 = sext i32 %296 to i64
  %298 = getelementptr inbounds float addrspace(1)* %1, i64 %297
  %299 = extractelement <16 x i32> %274, i32 8
  %300 = sext i32 %299 to i64
  %301 = getelementptr inbounds float addrspace(1)* %1, i64 %300
  %302 = extractelement <16 x i32> %274, i32 9
  %303 = sext i32 %302 to i64
  %304 = getelementptr inbounds float addrspace(1)* %1, i64 %303
  %305 = extractelement <16 x i32> %274, i32 10
  %306 = sext i32 %305 to i64
  %307 = getelementptr inbounds float addrspace(1)* %1, i64 %306
  %308 = extractelement <16 x i32> %274, i32 11
  %309 = sext i32 %308 to i64
  %310 = getelementptr inbounds float addrspace(1)* %1, i64 %309
  %311 = extractelement <16 x i32> %274, i32 12
  %312 = sext i32 %311 to i64
  %313 = getelementptr inbounds float addrspace(1)* %1, i64 %312
  %314 = extractelement <16 x i32> %274, i32 13
  %315 = sext i32 %314 to i64
  %316 = getelementptr inbounds float addrspace(1)* %1, i64 %315
  %317 = extractelement <16 x i32> %274, i32 14
  %318 = sext i32 %317 to i64
  %319 = getelementptr inbounds float addrspace(1)* %1, i64 %318
  %320 = extractelement <16 x i32> %274, i32 15
  %321 = sext i32 %320 to i64
  %322 = getelementptr inbounds float addrspace(1)* %1, i64 %321
  store float %extract16.i, float addrspace(1)* %277, align 4
  store float %extract17.i, float addrspace(1)* %280, align 4
  store float %extract18.i, float addrspace(1)* %283, align 4
  store float %extract19.i, float addrspace(1)* %286, align 4
  store float %extract20.i, float addrspace(1)* %289, align 4
  store float %extract21.i, float addrspace(1)* %292, align 4
  store float %extract22.i, float addrspace(1)* %295, align 4
  store float %extract23.i, float addrspace(1)* %298, align 4
  store float %extract24.i, float addrspace(1)* %301, align 4
  store float %extract25.i, float addrspace(1)* %304, align 4
  store float %extract26.i, float addrspace(1)* %307, align 4
  store float %extract27.i, float addrspace(1)* %310, align 4
  store float %extract28.i, float addrspace(1)* %313, align 4
  store float %extract29.i, float addrspace(1)* %316, align 4
  store float %extract30.i, float addrspace(1)* %319, align 4
  store float %extract31.i, float addrspace(1)* %322, align 4
  %323 = add nsw <16 x i32> %vectorPHI.i, <i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192, i32 192>
  %324 = and <16 x i32> %323, %vector.i
  %325 = extractelement <16 x i32> %324, i32 0
  %326 = sext i32 %325 to i64
  %327 = getelementptr inbounds float addrspace(1)* %1, i64 %326
  %328 = extractelement <16 x i32> %324, i32 1
  %329 = sext i32 %328 to i64
  %330 = getelementptr inbounds float addrspace(1)* %1, i64 %329
  %331 = extractelement <16 x i32> %324, i32 2
  %332 = sext i32 %331 to i64
  %333 = getelementptr inbounds float addrspace(1)* %1, i64 %332
  %334 = extractelement <16 x i32> %324, i32 3
  %335 = sext i32 %334 to i64
  %336 = getelementptr inbounds float addrspace(1)* %1, i64 %335
  %337 = extractelement <16 x i32> %324, i32 4
  %338 = sext i32 %337 to i64
  %339 = getelementptr inbounds float addrspace(1)* %1, i64 %338
  %340 = extractelement <16 x i32> %324, i32 5
  %341 = sext i32 %340 to i64
  %342 = getelementptr inbounds float addrspace(1)* %1, i64 %341
  %343 = extractelement <16 x i32> %324, i32 6
  %344 = sext i32 %343 to i64
  %345 = getelementptr inbounds float addrspace(1)* %1, i64 %344
  %346 = extractelement <16 x i32> %324, i32 7
  %347 = sext i32 %346 to i64
  %348 = getelementptr inbounds float addrspace(1)* %1, i64 %347
  %349 = extractelement <16 x i32> %324, i32 8
  %350 = sext i32 %349 to i64
  %351 = getelementptr inbounds float addrspace(1)* %1, i64 %350
  %352 = extractelement <16 x i32> %324, i32 9
  %353 = sext i32 %352 to i64
  %354 = getelementptr inbounds float addrspace(1)* %1, i64 %353
  %355 = extractelement <16 x i32> %324, i32 10
  %356 = sext i32 %355 to i64
  %357 = getelementptr inbounds float addrspace(1)* %1, i64 %356
  %358 = extractelement <16 x i32> %324, i32 11
  %359 = sext i32 %358 to i64
  %360 = getelementptr inbounds float addrspace(1)* %1, i64 %359
  %361 = extractelement <16 x i32> %324, i32 12
  %362 = sext i32 %361 to i64
  %363 = getelementptr inbounds float addrspace(1)* %1, i64 %362
  %364 = extractelement <16 x i32> %324, i32 13
  %365 = sext i32 %364 to i64
  %366 = getelementptr inbounds float addrspace(1)* %1, i64 %365
  %367 = extractelement <16 x i32> %324, i32 14
  %368 = sext i32 %367 to i64
  %369 = getelementptr inbounds float addrspace(1)* %1, i64 %368
  %370 = extractelement <16 x i32> %324, i32 15
  %371 = sext i32 %370 to i64
  %372 = getelementptr inbounds float addrspace(1)* %1, i64 %371
  store float %extract16.i, float addrspace(1)* %327, align 4
  store float %extract17.i, float addrspace(1)* %330, align 4
  store float %extract18.i, float addrspace(1)* %333, align 4
  store float %extract19.i, float addrspace(1)* %336, align 4
  store float %extract20.i, float addrspace(1)* %339, align 4
  store float %extract21.i, float addrspace(1)* %342, align 4
  store float %extract22.i, float addrspace(1)* %345, align 4
  store float %extract23.i, float addrspace(1)* %348, align 4
  store float %extract24.i, float addrspace(1)* %351, align 4
  store float %extract25.i, float addrspace(1)* %354, align 4
  store float %extract26.i, float addrspace(1)* %357, align 4
  store float %extract27.i, float addrspace(1)* %360, align 4
  store float %extract28.i, float addrspace(1)* %363, align 4
  store float %extract29.i, float addrspace(1)* %366, align 4
  store float %extract30.i, float addrspace(1)* %369, align 4
  store float %extract31.i, float addrspace(1)* %372, align 4
  %373 = add nsw <16 x i32> %vectorPHI.i, <i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224, i32 224>
  %374 = and <16 x i32> %373, %vector.i
  %375 = extractelement <16 x i32> %374, i32 0
  %376 = sext i32 %375 to i64
  %377 = getelementptr inbounds float addrspace(1)* %1, i64 %376
  %378 = extractelement <16 x i32> %374, i32 1
  %379 = sext i32 %378 to i64
  %380 = getelementptr inbounds float addrspace(1)* %1, i64 %379
  %381 = extractelement <16 x i32> %374, i32 2
  %382 = sext i32 %381 to i64
  %383 = getelementptr inbounds float addrspace(1)* %1, i64 %382
  %384 = extractelement <16 x i32> %374, i32 3
  %385 = sext i32 %384 to i64
  %386 = getelementptr inbounds float addrspace(1)* %1, i64 %385
  %387 = extractelement <16 x i32> %374, i32 4
  %388 = sext i32 %387 to i64
  %389 = getelementptr inbounds float addrspace(1)* %1, i64 %388
  %390 = extractelement <16 x i32> %374, i32 5
  %391 = sext i32 %390 to i64
  %392 = getelementptr inbounds float addrspace(1)* %1, i64 %391
  %393 = extractelement <16 x i32> %374, i32 6
  %394 = sext i32 %393 to i64
  %395 = getelementptr inbounds float addrspace(1)* %1, i64 %394
  %396 = extractelement <16 x i32> %374, i32 7
  %397 = sext i32 %396 to i64
  %398 = getelementptr inbounds float addrspace(1)* %1, i64 %397
  %399 = extractelement <16 x i32> %374, i32 8
  %400 = sext i32 %399 to i64
  %401 = getelementptr inbounds float addrspace(1)* %1, i64 %400
  %402 = extractelement <16 x i32> %374, i32 9
  %403 = sext i32 %402 to i64
  %404 = getelementptr inbounds float addrspace(1)* %1, i64 %403
  %405 = extractelement <16 x i32> %374, i32 10
  %406 = sext i32 %405 to i64
  %407 = getelementptr inbounds float addrspace(1)* %1, i64 %406
  %408 = extractelement <16 x i32> %374, i32 11
  %409 = sext i32 %408 to i64
  %410 = getelementptr inbounds float addrspace(1)* %1, i64 %409
  %411 = extractelement <16 x i32> %374, i32 12
  %412 = sext i32 %411 to i64
  %413 = getelementptr inbounds float addrspace(1)* %1, i64 %412
  %414 = extractelement <16 x i32> %374, i32 13
  %415 = sext i32 %414 to i64
  %416 = getelementptr inbounds float addrspace(1)* %1, i64 %415
  %417 = extractelement <16 x i32> %374, i32 14
  %418 = sext i32 %417 to i64
  %419 = getelementptr inbounds float addrspace(1)* %1, i64 %418
  %420 = extractelement <16 x i32> %374, i32 15
  %421 = sext i32 %420 to i64
  %422 = getelementptr inbounds float addrspace(1)* %1, i64 %421
  store float %extract16.i, float addrspace(1)* %377, align 4
  store float %extract17.i, float addrspace(1)* %380, align 4
  store float %extract18.i, float addrspace(1)* %383, align 4
  store float %extract19.i, float addrspace(1)* %386, align 4
  store float %extract20.i, float addrspace(1)* %389, align 4
  store float %extract21.i, float addrspace(1)* %392, align 4
  store float %extract22.i, float addrspace(1)* %395, align 4
  store float %extract23.i, float addrspace(1)* %398, align 4
  store float %extract24.i, float addrspace(1)* %401, align 4
  store float %extract25.i, float addrspace(1)* %404, align 4
  store float %extract26.i, float addrspace(1)* %407, align 4
  store float %extract27.i, float addrspace(1)* %410, align 4
  store float %extract28.i, float addrspace(1)* %413, align 4
  store float %extract29.i, float addrspace(1)* %416, align 4
  store float %extract30.i, float addrspace(1)* %419, align 4
  store float %extract31.i, float addrspace(1)* %422, align 4
  %423 = add nsw <16 x i32> %vectorPHI.i, <i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256, i32 256>
  %424 = and <16 x i32> %423, %vector.i
  %425 = extractelement <16 x i32> %424, i32 0
  %426 = sext i32 %425 to i64
  %427 = getelementptr inbounds float addrspace(1)* %1, i64 %426
  %428 = extractelement <16 x i32> %424, i32 1
  %429 = sext i32 %428 to i64
  %430 = getelementptr inbounds float addrspace(1)* %1, i64 %429
  %431 = extractelement <16 x i32> %424, i32 2
  %432 = sext i32 %431 to i64
  %433 = getelementptr inbounds float addrspace(1)* %1, i64 %432
  %434 = extractelement <16 x i32> %424, i32 3
  %435 = sext i32 %434 to i64
  %436 = getelementptr inbounds float addrspace(1)* %1, i64 %435
  %437 = extractelement <16 x i32> %424, i32 4
  %438 = sext i32 %437 to i64
  %439 = getelementptr inbounds float addrspace(1)* %1, i64 %438
  %440 = extractelement <16 x i32> %424, i32 5
  %441 = sext i32 %440 to i64
  %442 = getelementptr inbounds float addrspace(1)* %1, i64 %441
  %443 = extractelement <16 x i32> %424, i32 6
  %444 = sext i32 %443 to i64
  %445 = getelementptr inbounds float addrspace(1)* %1, i64 %444
  %446 = extractelement <16 x i32> %424, i32 7
  %447 = sext i32 %446 to i64
  %448 = getelementptr inbounds float addrspace(1)* %1, i64 %447
  %449 = extractelement <16 x i32> %424, i32 8
  %450 = sext i32 %449 to i64
  %451 = getelementptr inbounds float addrspace(1)* %1, i64 %450
  %452 = extractelement <16 x i32> %424, i32 9
  %453 = sext i32 %452 to i64
  %454 = getelementptr inbounds float addrspace(1)* %1, i64 %453
  %455 = extractelement <16 x i32> %424, i32 10
  %456 = sext i32 %455 to i64
  %457 = getelementptr inbounds float addrspace(1)* %1, i64 %456
  %458 = extractelement <16 x i32> %424, i32 11
  %459 = sext i32 %458 to i64
  %460 = getelementptr inbounds float addrspace(1)* %1, i64 %459
  %461 = extractelement <16 x i32> %424, i32 12
  %462 = sext i32 %461 to i64
  %463 = getelementptr inbounds float addrspace(1)* %1, i64 %462
  %464 = extractelement <16 x i32> %424, i32 13
  %465 = sext i32 %464 to i64
  %466 = getelementptr inbounds float addrspace(1)* %1, i64 %465
  %467 = extractelement <16 x i32> %424, i32 14
  %468 = sext i32 %467 to i64
  %469 = getelementptr inbounds float addrspace(1)* %1, i64 %468
  %470 = extractelement <16 x i32> %424, i32 15
  %471 = sext i32 %470 to i64
  %472 = getelementptr inbounds float addrspace(1)* %1, i64 %471
  store float %extract16.i, float addrspace(1)* %427, align 4
  store float %extract17.i, float addrspace(1)* %430, align 4
  store float %extract18.i, float addrspace(1)* %433, align 4
  store float %extract19.i, float addrspace(1)* %436, align 4
  store float %extract20.i, float addrspace(1)* %439, align 4
  store float %extract21.i, float addrspace(1)* %442, align 4
  store float %extract22.i, float addrspace(1)* %445, align 4
  store float %extract23.i, float addrspace(1)* %448, align 4
  store float %extract24.i, float addrspace(1)* %451, align 4
  store float %extract25.i, float addrspace(1)* %454, align 4
  store float %extract26.i, float addrspace(1)* %457, align 4
  store float %extract27.i, float addrspace(1)* %460, align 4
  store float %extract28.i, float addrspace(1)* %463, align 4
  store float %extract29.i, float addrspace(1)* %466, align 4
  store float %extract30.i, float addrspace(1)* %469, align 4
  store float %extract31.i, float addrspace(1)* %472, align 4
  %473 = add nsw <16 x i32> %vectorPHI.i, <i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288, i32 288>
  %474 = and <16 x i32> %473, %vector.i
  %475 = extractelement <16 x i32> %474, i32 0
  %476 = sext i32 %475 to i64
  %477 = getelementptr inbounds float addrspace(1)* %1, i64 %476
  %478 = extractelement <16 x i32> %474, i32 1
  %479 = sext i32 %478 to i64
  %480 = getelementptr inbounds float addrspace(1)* %1, i64 %479
  %481 = extractelement <16 x i32> %474, i32 2
  %482 = sext i32 %481 to i64
  %483 = getelementptr inbounds float addrspace(1)* %1, i64 %482
  %484 = extractelement <16 x i32> %474, i32 3
  %485 = sext i32 %484 to i64
  %486 = getelementptr inbounds float addrspace(1)* %1, i64 %485
  %487 = extractelement <16 x i32> %474, i32 4
  %488 = sext i32 %487 to i64
  %489 = getelementptr inbounds float addrspace(1)* %1, i64 %488
  %490 = extractelement <16 x i32> %474, i32 5
  %491 = sext i32 %490 to i64
  %492 = getelementptr inbounds float addrspace(1)* %1, i64 %491
  %493 = extractelement <16 x i32> %474, i32 6
  %494 = sext i32 %493 to i64
  %495 = getelementptr inbounds float addrspace(1)* %1, i64 %494
  %496 = extractelement <16 x i32> %474, i32 7
  %497 = sext i32 %496 to i64
  %498 = getelementptr inbounds float addrspace(1)* %1, i64 %497
  %499 = extractelement <16 x i32> %474, i32 8
  %500 = sext i32 %499 to i64
  %501 = getelementptr inbounds float addrspace(1)* %1, i64 %500
  %502 = extractelement <16 x i32> %474, i32 9
  %503 = sext i32 %502 to i64
  %504 = getelementptr inbounds float addrspace(1)* %1, i64 %503
  %505 = extractelement <16 x i32> %474, i32 10
  %506 = sext i32 %505 to i64
  %507 = getelementptr inbounds float addrspace(1)* %1, i64 %506
  %508 = extractelement <16 x i32> %474, i32 11
  %509 = sext i32 %508 to i64
  %510 = getelementptr inbounds float addrspace(1)* %1, i64 %509
  %511 = extractelement <16 x i32> %474, i32 12
  %512 = sext i32 %511 to i64
  %513 = getelementptr inbounds float addrspace(1)* %1, i64 %512
  %514 = extractelement <16 x i32> %474, i32 13
  %515 = sext i32 %514 to i64
  %516 = getelementptr inbounds float addrspace(1)* %1, i64 %515
  %517 = extractelement <16 x i32> %474, i32 14
  %518 = sext i32 %517 to i64
  %519 = getelementptr inbounds float addrspace(1)* %1, i64 %518
  %520 = extractelement <16 x i32> %474, i32 15
  %521 = sext i32 %520 to i64
  %522 = getelementptr inbounds float addrspace(1)* %1, i64 %521
  store float %extract16.i, float addrspace(1)* %477, align 4
  store float %extract17.i, float addrspace(1)* %480, align 4
  store float %extract18.i, float addrspace(1)* %483, align 4
  store float %extract19.i, float addrspace(1)* %486, align 4
  store float %extract20.i, float addrspace(1)* %489, align 4
  store float %extract21.i, float addrspace(1)* %492, align 4
  store float %extract22.i, float addrspace(1)* %495, align 4
  store float %extract23.i, float addrspace(1)* %498, align 4
  store float %extract24.i, float addrspace(1)* %501, align 4
  store float %extract25.i, float addrspace(1)* %504, align 4
  store float %extract26.i, float addrspace(1)* %507, align 4
  store float %extract27.i, float addrspace(1)* %510, align 4
  store float %extract28.i, float addrspace(1)* %513, align 4
  store float %extract29.i, float addrspace(1)* %516, align 4
  store float %extract30.i, float addrspace(1)* %519, align 4
  store float %extract31.i, float addrspace(1)* %522, align 4
  %523 = add nsw <16 x i32> %vectorPHI.i, <i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320, i32 320>
  %524 = and <16 x i32> %523, %vector.i
  %525 = extractelement <16 x i32> %524, i32 0
  %526 = sext i32 %525 to i64
  %527 = getelementptr inbounds float addrspace(1)* %1, i64 %526
  %528 = extractelement <16 x i32> %524, i32 1
  %529 = sext i32 %528 to i64
  %530 = getelementptr inbounds float addrspace(1)* %1, i64 %529
  %531 = extractelement <16 x i32> %524, i32 2
  %532 = sext i32 %531 to i64
  %533 = getelementptr inbounds float addrspace(1)* %1, i64 %532
  %534 = extractelement <16 x i32> %524, i32 3
  %535 = sext i32 %534 to i64
  %536 = getelementptr inbounds float addrspace(1)* %1, i64 %535
  %537 = extractelement <16 x i32> %524, i32 4
  %538 = sext i32 %537 to i64
  %539 = getelementptr inbounds float addrspace(1)* %1, i64 %538
  %540 = extractelement <16 x i32> %524, i32 5
  %541 = sext i32 %540 to i64
  %542 = getelementptr inbounds float addrspace(1)* %1, i64 %541
  %543 = extractelement <16 x i32> %524, i32 6
  %544 = sext i32 %543 to i64
  %545 = getelementptr inbounds float addrspace(1)* %1, i64 %544
  %546 = extractelement <16 x i32> %524, i32 7
  %547 = sext i32 %546 to i64
  %548 = getelementptr inbounds float addrspace(1)* %1, i64 %547
  %549 = extractelement <16 x i32> %524, i32 8
  %550 = sext i32 %549 to i64
  %551 = getelementptr inbounds float addrspace(1)* %1, i64 %550
  %552 = extractelement <16 x i32> %524, i32 9
  %553 = sext i32 %552 to i64
  %554 = getelementptr inbounds float addrspace(1)* %1, i64 %553
  %555 = extractelement <16 x i32> %524, i32 10
  %556 = sext i32 %555 to i64
  %557 = getelementptr inbounds float addrspace(1)* %1, i64 %556
  %558 = extractelement <16 x i32> %524, i32 11
  %559 = sext i32 %558 to i64
  %560 = getelementptr inbounds float addrspace(1)* %1, i64 %559
  %561 = extractelement <16 x i32> %524, i32 12
  %562 = sext i32 %561 to i64
  %563 = getelementptr inbounds float addrspace(1)* %1, i64 %562
  %564 = extractelement <16 x i32> %524, i32 13
  %565 = sext i32 %564 to i64
  %566 = getelementptr inbounds float addrspace(1)* %1, i64 %565
  %567 = extractelement <16 x i32> %524, i32 14
  %568 = sext i32 %567 to i64
  %569 = getelementptr inbounds float addrspace(1)* %1, i64 %568
  %570 = extractelement <16 x i32> %524, i32 15
  %571 = sext i32 %570 to i64
  %572 = getelementptr inbounds float addrspace(1)* %1, i64 %571
  store float %extract16.i, float addrspace(1)* %527, align 4
  store float %extract17.i, float addrspace(1)* %530, align 4
  store float %extract18.i, float addrspace(1)* %533, align 4
  store float %extract19.i, float addrspace(1)* %536, align 4
  store float %extract20.i, float addrspace(1)* %539, align 4
  store float %extract21.i, float addrspace(1)* %542, align 4
  store float %extract22.i, float addrspace(1)* %545, align 4
  store float %extract23.i, float addrspace(1)* %548, align 4
  store float %extract24.i, float addrspace(1)* %551, align 4
  store float %extract25.i, float addrspace(1)* %554, align 4
  store float %extract26.i, float addrspace(1)* %557, align 4
  store float %extract27.i, float addrspace(1)* %560, align 4
  store float %extract28.i, float addrspace(1)* %563, align 4
  store float %extract29.i, float addrspace(1)* %566, align 4
  store float %extract30.i, float addrspace(1)* %569, align 4
  store float %extract31.i, float addrspace(1)* %572, align 4
  %573 = add nsw <16 x i32> %vectorPHI.i, <i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352, i32 352>
  %574 = and <16 x i32> %573, %vector.i
  %575 = extractelement <16 x i32> %574, i32 0
  %576 = sext i32 %575 to i64
  %577 = getelementptr inbounds float addrspace(1)* %1, i64 %576
  %578 = extractelement <16 x i32> %574, i32 1
  %579 = sext i32 %578 to i64
  %580 = getelementptr inbounds float addrspace(1)* %1, i64 %579
  %581 = extractelement <16 x i32> %574, i32 2
  %582 = sext i32 %581 to i64
  %583 = getelementptr inbounds float addrspace(1)* %1, i64 %582
  %584 = extractelement <16 x i32> %574, i32 3
  %585 = sext i32 %584 to i64
  %586 = getelementptr inbounds float addrspace(1)* %1, i64 %585
  %587 = extractelement <16 x i32> %574, i32 4
  %588 = sext i32 %587 to i64
  %589 = getelementptr inbounds float addrspace(1)* %1, i64 %588
  %590 = extractelement <16 x i32> %574, i32 5
  %591 = sext i32 %590 to i64
  %592 = getelementptr inbounds float addrspace(1)* %1, i64 %591
  %593 = extractelement <16 x i32> %574, i32 6
  %594 = sext i32 %593 to i64
  %595 = getelementptr inbounds float addrspace(1)* %1, i64 %594
  %596 = extractelement <16 x i32> %574, i32 7
  %597 = sext i32 %596 to i64
  %598 = getelementptr inbounds float addrspace(1)* %1, i64 %597
  %599 = extractelement <16 x i32> %574, i32 8
  %600 = sext i32 %599 to i64
  %601 = getelementptr inbounds float addrspace(1)* %1, i64 %600
  %602 = extractelement <16 x i32> %574, i32 9
  %603 = sext i32 %602 to i64
  %604 = getelementptr inbounds float addrspace(1)* %1, i64 %603
  %605 = extractelement <16 x i32> %574, i32 10
  %606 = sext i32 %605 to i64
  %607 = getelementptr inbounds float addrspace(1)* %1, i64 %606
  %608 = extractelement <16 x i32> %574, i32 11
  %609 = sext i32 %608 to i64
  %610 = getelementptr inbounds float addrspace(1)* %1, i64 %609
  %611 = extractelement <16 x i32> %574, i32 12
  %612 = sext i32 %611 to i64
  %613 = getelementptr inbounds float addrspace(1)* %1, i64 %612
  %614 = extractelement <16 x i32> %574, i32 13
  %615 = sext i32 %614 to i64
  %616 = getelementptr inbounds float addrspace(1)* %1, i64 %615
  %617 = extractelement <16 x i32> %574, i32 14
  %618 = sext i32 %617 to i64
  %619 = getelementptr inbounds float addrspace(1)* %1, i64 %618
  %620 = extractelement <16 x i32> %574, i32 15
  %621 = sext i32 %620 to i64
  %622 = getelementptr inbounds float addrspace(1)* %1, i64 %621
  store float %extract16.i, float addrspace(1)* %577, align 4
  store float %extract17.i, float addrspace(1)* %580, align 4
  store float %extract18.i, float addrspace(1)* %583, align 4
  store float %extract19.i, float addrspace(1)* %586, align 4
  store float %extract20.i, float addrspace(1)* %589, align 4
  store float %extract21.i, float addrspace(1)* %592, align 4
  store float %extract22.i, float addrspace(1)* %595, align 4
  store float %extract23.i, float addrspace(1)* %598, align 4
  store float %extract24.i, float addrspace(1)* %601, align 4
  store float %extract25.i, float addrspace(1)* %604, align 4
  store float %extract26.i, float addrspace(1)* %607, align 4
  store float %extract27.i, float addrspace(1)* %610, align 4
  store float %extract28.i, float addrspace(1)* %613, align 4
  store float %extract29.i, float addrspace(1)* %616, align 4
  store float %extract30.i, float addrspace(1)* %619, align 4
  store float %extract31.i, float addrspace(1)* %622, align 4
  %623 = add nsw <16 x i32> %vectorPHI.i, <i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384, i32 384>
  %624 = and <16 x i32> %623, %vector.i
  %625 = extractelement <16 x i32> %624, i32 0
  %626 = sext i32 %625 to i64
  %627 = getelementptr inbounds float addrspace(1)* %1, i64 %626
  %628 = extractelement <16 x i32> %624, i32 1
  %629 = sext i32 %628 to i64
  %630 = getelementptr inbounds float addrspace(1)* %1, i64 %629
  %631 = extractelement <16 x i32> %624, i32 2
  %632 = sext i32 %631 to i64
  %633 = getelementptr inbounds float addrspace(1)* %1, i64 %632
  %634 = extractelement <16 x i32> %624, i32 3
  %635 = sext i32 %634 to i64
  %636 = getelementptr inbounds float addrspace(1)* %1, i64 %635
  %637 = extractelement <16 x i32> %624, i32 4
  %638 = sext i32 %637 to i64
  %639 = getelementptr inbounds float addrspace(1)* %1, i64 %638
  %640 = extractelement <16 x i32> %624, i32 5
  %641 = sext i32 %640 to i64
  %642 = getelementptr inbounds float addrspace(1)* %1, i64 %641
  %643 = extractelement <16 x i32> %624, i32 6
  %644 = sext i32 %643 to i64
  %645 = getelementptr inbounds float addrspace(1)* %1, i64 %644
  %646 = extractelement <16 x i32> %624, i32 7
  %647 = sext i32 %646 to i64
  %648 = getelementptr inbounds float addrspace(1)* %1, i64 %647
  %649 = extractelement <16 x i32> %624, i32 8
  %650 = sext i32 %649 to i64
  %651 = getelementptr inbounds float addrspace(1)* %1, i64 %650
  %652 = extractelement <16 x i32> %624, i32 9
  %653 = sext i32 %652 to i64
  %654 = getelementptr inbounds float addrspace(1)* %1, i64 %653
  %655 = extractelement <16 x i32> %624, i32 10
  %656 = sext i32 %655 to i64
  %657 = getelementptr inbounds float addrspace(1)* %1, i64 %656
  %658 = extractelement <16 x i32> %624, i32 11
  %659 = sext i32 %658 to i64
  %660 = getelementptr inbounds float addrspace(1)* %1, i64 %659
  %661 = extractelement <16 x i32> %624, i32 12
  %662 = sext i32 %661 to i64
  %663 = getelementptr inbounds float addrspace(1)* %1, i64 %662
  %664 = extractelement <16 x i32> %624, i32 13
  %665 = sext i32 %664 to i64
  %666 = getelementptr inbounds float addrspace(1)* %1, i64 %665
  %667 = extractelement <16 x i32> %624, i32 14
  %668 = sext i32 %667 to i64
  %669 = getelementptr inbounds float addrspace(1)* %1, i64 %668
  %670 = extractelement <16 x i32> %624, i32 15
  %671 = sext i32 %670 to i64
  %672 = getelementptr inbounds float addrspace(1)* %1, i64 %671
  store float %extract16.i, float addrspace(1)* %627, align 4
  store float %extract17.i, float addrspace(1)* %630, align 4
  store float %extract18.i, float addrspace(1)* %633, align 4
  store float %extract19.i, float addrspace(1)* %636, align 4
  store float %extract20.i, float addrspace(1)* %639, align 4
  store float %extract21.i, float addrspace(1)* %642, align 4
  store float %extract22.i, float addrspace(1)* %645, align 4
  store float %extract23.i, float addrspace(1)* %648, align 4
  store float %extract24.i, float addrspace(1)* %651, align 4
  store float %extract25.i, float addrspace(1)* %654, align 4
  store float %extract26.i, float addrspace(1)* %657, align 4
  store float %extract27.i, float addrspace(1)* %660, align 4
  store float %extract28.i, float addrspace(1)* %663, align 4
  store float %extract29.i, float addrspace(1)* %666, align 4
  store float %extract30.i, float addrspace(1)* %669, align 4
  store float %extract31.i, float addrspace(1)* %672, align 4
  %673 = add nsw <16 x i32> %vectorPHI.i, <i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416, i32 416>
  %674 = and <16 x i32> %673, %vector.i
  %675 = extractelement <16 x i32> %674, i32 0
  %676 = sext i32 %675 to i64
  %677 = getelementptr inbounds float addrspace(1)* %1, i64 %676
  %678 = extractelement <16 x i32> %674, i32 1
  %679 = sext i32 %678 to i64
  %680 = getelementptr inbounds float addrspace(1)* %1, i64 %679
  %681 = extractelement <16 x i32> %674, i32 2
  %682 = sext i32 %681 to i64
  %683 = getelementptr inbounds float addrspace(1)* %1, i64 %682
  %684 = extractelement <16 x i32> %674, i32 3
  %685 = sext i32 %684 to i64
  %686 = getelementptr inbounds float addrspace(1)* %1, i64 %685
  %687 = extractelement <16 x i32> %674, i32 4
  %688 = sext i32 %687 to i64
  %689 = getelementptr inbounds float addrspace(1)* %1, i64 %688
  %690 = extractelement <16 x i32> %674, i32 5
  %691 = sext i32 %690 to i64
  %692 = getelementptr inbounds float addrspace(1)* %1, i64 %691
  %693 = extractelement <16 x i32> %674, i32 6
  %694 = sext i32 %693 to i64
  %695 = getelementptr inbounds float addrspace(1)* %1, i64 %694
  %696 = extractelement <16 x i32> %674, i32 7
  %697 = sext i32 %696 to i64
  %698 = getelementptr inbounds float addrspace(1)* %1, i64 %697
  %699 = extractelement <16 x i32> %674, i32 8
  %700 = sext i32 %699 to i64
  %701 = getelementptr inbounds float addrspace(1)* %1, i64 %700
  %702 = extractelement <16 x i32> %674, i32 9
  %703 = sext i32 %702 to i64
  %704 = getelementptr inbounds float addrspace(1)* %1, i64 %703
  %705 = extractelement <16 x i32> %674, i32 10
  %706 = sext i32 %705 to i64
  %707 = getelementptr inbounds float addrspace(1)* %1, i64 %706
  %708 = extractelement <16 x i32> %674, i32 11
  %709 = sext i32 %708 to i64
  %710 = getelementptr inbounds float addrspace(1)* %1, i64 %709
  %711 = extractelement <16 x i32> %674, i32 12
  %712 = sext i32 %711 to i64
  %713 = getelementptr inbounds float addrspace(1)* %1, i64 %712
  %714 = extractelement <16 x i32> %674, i32 13
  %715 = sext i32 %714 to i64
  %716 = getelementptr inbounds float addrspace(1)* %1, i64 %715
  %717 = extractelement <16 x i32> %674, i32 14
  %718 = sext i32 %717 to i64
  %719 = getelementptr inbounds float addrspace(1)* %1, i64 %718
  %720 = extractelement <16 x i32> %674, i32 15
  %721 = sext i32 %720 to i64
  %722 = getelementptr inbounds float addrspace(1)* %1, i64 %721
  store float %extract16.i, float addrspace(1)* %677, align 4
  store float %extract17.i, float addrspace(1)* %680, align 4
  store float %extract18.i, float addrspace(1)* %683, align 4
  store float %extract19.i, float addrspace(1)* %686, align 4
  store float %extract20.i, float addrspace(1)* %689, align 4
  store float %extract21.i, float addrspace(1)* %692, align 4
  store float %extract22.i, float addrspace(1)* %695, align 4
  store float %extract23.i, float addrspace(1)* %698, align 4
  store float %extract24.i, float addrspace(1)* %701, align 4
  store float %extract25.i, float addrspace(1)* %704, align 4
  store float %extract26.i, float addrspace(1)* %707, align 4
  store float %extract27.i, float addrspace(1)* %710, align 4
  store float %extract28.i, float addrspace(1)* %713, align 4
  store float %extract29.i, float addrspace(1)* %716, align 4
  store float %extract30.i, float addrspace(1)* %719, align 4
  store float %extract31.i, float addrspace(1)* %722, align 4
  %723 = add nsw <16 x i32> %vectorPHI.i, <i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448, i32 448>
  %724 = and <16 x i32> %723, %vector.i
  %725 = extractelement <16 x i32> %724, i32 0
  %726 = sext i32 %725 to i64
  %727 = getelementptr inbounds float addrspace(1)* %1, i64 %726
  %728 = extractelement <16 x i32> %724, i32 1
  %729 = sext i32 %728 to i64
  %730 = getelementptr inbounds float addrspace(1)* %1, i64 %729
  %731 = extractelement <16 x i32> %724, i32 2
  %732 = sext i32 %731 to i64
  %733 = getelementptr inbounds float addrspace(1)* %1, i64 %732
  %734 = extractelement <16 x i32> %724, i32 3
  %735 = sext i32 %734 to i64
  %736 = getelementptr inbounds float addrspace(1)* %1, i64 %735
  %737 = extractelement <16 x i32> %724, i32 4
  %738 = sext i32 %737 to i64
  %739 = getelementptr inbounds float addrspace(1)* %1, i64 %738
  %740 = extractelement <16 x i32> %724, i32 5
  %741 = sext i32 %740 to i64
  %742 = getelementptr inbounds float addrspace(1)* %1, i64 %741
  %743 = extractelement <16 x i32> %724, i32 6
  %744 = sext i32 %743 to i64
  %745 = getelementptr inbounds float addrspace(1)* %1, i64 %744
  %746 = extractelement <16 x i32> %724, i32 7
  %747 = sext i32 %746 to i64
  %748 = getelementptr inbounds float addrspace(1)* %1, i64 %747
  %749 = extractelement <16 x i32> %724, i32 8
  %750 = sext i32 %749 to i64
  %751 = getelementptr inbounds float addrspace(1)* %1, i64 %750
  %752 = extractelement <16 x i32> %724, i32 9
  %753 = sext i32 %752 to i64
  %754 = getelementptr inbounds float addrspace(1)* %1, i64 %753
  %755 = extractelement <16 x i32> %724, i32 10
  %756 = sext i32 %755 to i64
  %757 = getelementptr inbounds float addrspace(1)* %1, i64 %756
  %758 = extractelement <16 x i32> %724, i32 11
  %759 = sext i32 %758 to i64
  %760 = getelementptr inbounds float addrspace(1)* %1, i64 %759
  %761 = extractelement <16 x i32> %724, i32 12
  %762 = sext i32 %761 to i64
  %763 = getelementptr inbounds float addrspace(1)* %1, i64 %762
  %764 = extractelement <16 x i32> %724, i32 13
  %765 = sext i32 %764 to i64
  %766 = getelementptr inbounds float addrspace(1)* %1, i64 %765
  %767 = extractelement <16 x i32> %724, i32 14
  %768 = sext i32 %767 to i64
  %769 = getelementptr inbounds float addrspace(1)* %1, i64 %768
  %770 = extractelement <16 x i32> %724, i32 15
  %771 = sext i32 %770 to i64
  %772 = getelementptr inbounds float addrspace(1)* %1, i64 %771
  store float %extract16.i, float addrspace(1)* %727, align 4
  store float %extract17.i, float addrspace(1)* %730, align 4
  store float %extract18.i, float addrspace(1)* %733, align 4
  store float %extract19.i, float addrspace(1)* %736, align 4
  store float %extract20.i, float addrspace(1)* %739, align 4
  store float %extract21.i, float addrspace(1)* %742, align 4
  store float %extract22.i, float addrspace(1)* %745, align 4
  store float %extract23.i, float addrspace(1)* %748, align 4
  store float %extract24.i, float addrspace(1)* %751, align 4
  store float %extract25.i, float addrspace(1)* %754, align 4
  store float %extract26.i, float addrspace(1)* %757, align 4
  store float %extract27.i, float addrspace(1)* %760, align 4
  store float %extract28.i, float addrspace(1)* %763, align 4
  store float %extract29.i, float addrspace(1)* %766, align 4
  store float %extract30.i, float addrspace(1)* %769, align 4
  store float %extract31.i, float addrspace(1)* %772, align 4
  %773 = add nsw <16 x i32> %vectorPHI.i, <i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480, i32 480>
  %774 = and <16 x i32> %773, %vector.i
  %775 = extractelement <16 x i32> %774, i32 0
  %776 = sext i32 %775 to i64
  %777 = getelementptr inbounds float addrspace(1)* %1, i64 %776
  %778 = extractelement <16 x i32> %774, i32 1
  %779 = sext i32 %778 to i64
  %780 = getelementptr inbounds float addrspace(1)* %1, i64 %779
  %781 = extractelement <16 x i32> %774, i32 2
  %782 = sext i32 %781 to i64
  %783 = getelementptr inbounds float addrspace(1)* %1, i64 %782
  %784 = extractelement <16 x i32> %774, i32 3
  %785 = sext i32 %784 to i64
  %786 = getelementptr inbounds float addrspace(1)* %1, i64 %785
  %787 = extractelement <16 x i32> %774, i32 4
  %788 = sext i32 %787 to i64
  %789 = getelementptr inbounds float addrspace(1)* %1, i64 %788
  %790 = extractelement <16 x i32> %774, i32 5
  %791 = sext i32 %790 to i64
  %792 = getelementptr inbounds float addrspace(1)* %1, i64 %791
  %793 = extractelement <16 x i32> %774, i32 6
  %794 = sext i32 %793 to i64
  %795 = getelementptr inbounds float addrspace(1)* %1, i64 %794
  %796 = extractelement <16 x i32> %774, i32 7
  %797 = sext i32 %796 to i64
  %798 = getelementptr inbounds float addrspace(1)* %1, i64 %797
  %799 = extractelement <16 x i32> %774, i32 8
  %800 = sext i32 %799 to i64
  %801 = getelementptr inbounds float addrspace(1)* %1, i64 %800
  %802 = extractelement <16 x i32> %774, i32 9
  %803 = sext i32 %802 to i64
  %804 = getelementptr inbounds float addrspace(1)* %1, i64 %803
  %805 = extractelement <16 x i32> %774, i32 10
  %806 = sext i32 %805 to i64
  %807 = getelementptr inbounds float addrspace(1)* %1, i64 %806
  %808 = extractelement <16 x i32> %774, i32 11
  %809 = sext i32 %808 to i64
  %810 = getelementptr inbounds float addrspace(1)* %1, i64 %809
  %811 = extractelement <16 x i32> %774, i32 12
  %812 = sext i32 %811 to i64
  %813 = getelementptr inbounds float addrspace(1)* %1, i64 %812
  %814 = extractelement <16 x i32> %774, i32 13
  %815 = sext i32 %814 to i64
  %816 = getelementptr inbounds float addrspace(1)* %1, i64 %815
  %817 = extractelement <16 x i32> %774, i32 14
  %818 = sext i32 %817 to i64
  %819 = getelementptr inbounds float addrspace(1)* %1, i64 %818
  %820 = extractelement <16 x i32> %774, i32 15
  %821 = sext i32 %820 to i64
  %822 = getelementptr inbounds float addrspace(1)* %1, i64 %821
  store float %extract16.i, float addrspace(1)* %777, align 4
  store float %extract17.i, float addrspace(1)* %780, align 4
  store float %extract18.i, float addrspace(1)* %783, align 4
  store float %extract19.i, float addrspace(1)* %786, align 4
  store float %extract20.i, float addrspace(1)* %789, align 4
  store float %extract21.i, float addrspace(1)* %792, align 4
  store float %extract22.i, float addrspace(1)* %795, align 4
  store float %extract23.i, float addrspace(1)* %798, align 4
  store float %extract24.i, float addrspace(1)* %801, align 4
  store float %extract25.i, float addrspace(1)* %804, align 4
  store float %extract26.i, float addrspace(1)* %807, align 4
  store float %extract27.i, float addrspace(1)* %810, align 4
  store float %extract28.i, float addrspace(1)* %813, align 4
  store float %extract29.i, float addrspace(1)* %816, align 4
  store float %extract30.i, float addrspace(1)* %819, align 4
  store float %extract31.i, float addrspace(1)* %822, align 4
  %823 = add nsw i32 %j.01.i, 1
  %824 = add nsw <16 x i32> %vectorPHI.i, <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>
  %825 = and <16 x i32> %824, %vector.i
  %exitcond.i = icmp eq i32 %823, 1024
  br i1 %exitcond.i, label %._crit_edge.i, label %23

._crit_edge.i:                                    ; preds = %23
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %____Vectorized_.writeGlobalMemoryCoalesced_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

____Vectorized_.writeGlobalMemoryCoalesced_separated_args.exit: ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__writeGlobalMemoryCoalesced_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, int", metadata !"opencl_writeGlobalMemoryCoalesced_locals_anchor", void (i8*)* @writeGlobalMemoryCoalesced}
!1 = metadata !{i32 0, i32 0, i32 0}
