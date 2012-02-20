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

@opencl_readLocalMemory_local_lbuf = internal addrspace(3) global [4096 x float] zeroinitializer, align 16

declare void @__readLocalMemory_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

declare i64 @get_global_id(i32)

declare i64 @get_global_size(i32)

declare i64 @get_group_id(i32)

declare i64 @get_local_id(i32)

declare i64 @get_local_size(i32)

declare void @barrier(i64)

declare void @____Vectorized_.readLocalMemory_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, i32) nounwind

define i1 @allOne(i1 %pred) {
entry:
  ret i1 %pred
}

define i1 @allZero(i1 %t) {
entry:
  ret i1 %t
}

declare float @masked_load0(i1, float addrspace(1)*)

declare void @masked_store0(i1, float, float addrspace(3)*)

declare float @masked_load1(i1, float addrspace(1)*)

declare void @masked_store1(i1, float, float addrspace(3)*)

declare float @masked_load2(i1, float addrspace(3)*)

declare float @masked_load3(i1, float addrspace(3)*)

declare float @masked_load4(i1, float addrspace(3)*)

declare float @masked_load5(i1, float addrspace(3)*)

declare float @masked_load6(i1, float addrspace(3)*)

declare float @masked_load7(i1, float addrspace(3)*)

declare float @masked_load8(i1, float addrspace(3)*)

declare float @masked_load9(i1, float addrspace(3)*)

declare float @masked_load10(i1, float addrspace(3)*)

declare float @masked_load11(i1, float addrspace(3)*)

declare float @masked_load12(i1, float addrspace(3)*)

declare float @masked_load13(i1, float addrspace(3)*)

declare float @masked_load14(i1, float addrspace(3)*)

declare float @masked_load15(i1, float addrspace(3)*)

declare float @masked_load16(i1, float addrspace(3)*)

declare float @masked_load17(i1, float addrspace(3)*)

define i1 @allZero_v16(<16 x i1> %t) {
entry:
  %ipred = bitcast <16 x i1> %t to i16
  %val = call i32 @llvm.x86.mic.kortestz(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

define i1 @allOne_v16(<16 x i1> %pred) {
entry:
  %ipred = bitcast <16 x i1> %pred to i16
  %val = call i32 @llvm.x86.mic.kortestc(i16 %ipred, i16 %ipred)
  %tmp = and i32 %val, 1
  %res = icmp ne i32 %tmp, 0
  ret i1 %res
}

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__readLocalMemory_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  br label %SyncBB49

SyncBB49:                                         ; preds = %0, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %0 ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %6, i64* %CastToValueType, align 8
  %7 = load i64* %pWGId, align 8
  %8 = trunc i64 %7 to i32
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %"&(pSB[currWI].offset)411" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset42" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)411"
  %CastToValueType43 = bitcast i8* %"&pSB[currWI].offset42" to i32*
  store i32 %11, i32* %CastToValueType43, align 4
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %13 = load i64* %12, align 8
  %14 = trunc i64 %13 to i32
  %15 = icmp eq i32 %14, 0
  %16 = select i1 %15, i32 1, i32 %14
  %17 = sdiv i32 4096, %16
  %18 = mul nsw i32 %14, %8
  %19 = mul nsw i32 %17, %11
  %20 = add nsw i32 %18, %19
  %21 = sub nsw i32 %size, %20
  %22 = icmp sgt i32 %17, 0
  %23 = icmp sgt i32 %21, 0
  %or.cond7 = and i1 %22, %23
  br i1 %or.cond7, label %bb.nph9, label %.critedge.preheader

.critedge.preheader:                              ; preds = %25, %SyncBB49
  %j.0.lcssa = phi i32 [ 0, %SyncBB49 ], [ %tmp31, %25 ]
  %24 = icmp slt i32 %j.0.lcssa, %17
  br i1 %24, label %bb.nph6, label %bb.nph

bb.nph9:                                          ; preds = %SyncBB49
  %tmp22 = sub i32 0, %17
  %tmp28 = add i32 %19, %18
  %tmp29 = sub i32 %tmp28, %size
  %tmp30 = icmp ult i32 %tmp29, %tmp22
  %umax = select i1 %tmp30, i32 %tmp22, i32 %tmp29
  %tmp31 = sub i32 0, %umax
  br label %25

; <label>:25                                      ; preds = %25, %bb.nph9
  %j.08 = phi i32 [ 0, %bb.nph9 ], [ %31, %25 ]
  %tmp33 = add i32 %tmp28, %j.08
  %tmp34 = add i32 %19, %j.08
  %26 = sext i32 %tmp33 to i64
  %27 = getelementptr inbounds float addrspace(1)* %data, i64 %26
  %28 = load float addrspace(1)* %27, align 4
  %29 = sext i32 %tmp34 to i64
  %30 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %29
  store float %28, float addrspace(3)* %30, align 4
  %31 = add nsw i32 %j.08, 1
  %exitcond32 = icmp eq i32 %31, %tmp31
  br i1 %exitcond32, label %.critedge.preheader, label %25

bb.nph6:                                          ; preds = %.critedge.preheader
  %tmp = add i32 %17, -1
  %tmp11 = sub i32 %tmp, %j.0.lcssa
  %tmp12 = zext i32 %tmp11 to i64
  %tmp13 = add i64 %tmp12, 1
  %tmp17 = add i32 %j.0.lcssa, %19
  %tmp18 = zext i32 %tmp17 to i64
  br label %.critedge

.critedge:                                        ; preds = %.critedge, %bb.nph6
  %indvar = phi i64 [ 0, %bb.nph6 ], [ %indvar.next, %.critedge ]
  %scevgep = getelementptr float addrspace(1)* %data, i64 %indvar
  %tmp19 = add i64 %tmp18, %indvar
  %32 = load float addrspace(1)* %scevgep, align 4
  %sext35 = shl i64 %tmp19, 32
  %33 = ashr i64 %sext35, 32
  %34 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %33
  store float %32, float addrspace(3)* %34, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond14 = icmp eq i64 %indvar.next, %tmp13
  br i1 %exitcond14, label %bb.nph, label %.critedge

bb.nph:                                           ; preds = %.critedge, %.critedge.preheader
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %bb.nph
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 128
  br label %SyncBB49

SyncBB:                                           ; preds = %bb.nph, %thenBB52
  %CurrWI..1 = phi i64 [ %"CurrWI++56", %thenBB52 ], [ 0, %bb.nph ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride58", %thenBB52 ], [ 0, %bb.nph ]
  %"&(pSB[currWI].offset)452" = or i64 %CurrSBIndex..1, 8
  %"&pSB[currWI].offset46" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)452"
  %CastToValueType47 = bitcast i8* %"&pSB[currWI].offset46" to i32*
  %loadedValue48 = load i32* %CastToValueType47, align 4
  br label %35

; <label>:35                                      ; preds = %35, %SyncBB
  %s.03 = phi i32 [ %loadedValue48, %SyncBB ], [ %133, %35 ]
  %sum.02 = phi float [ 0.000000e+00, %SyncBB ], [ %130, %35 ]
  %j.21 = phi i32 [ 0, %SyncBB ], [ %131, %35 ]
  %36 = and i32 %s.03, 4095
  %37 = zext i32 %36 to i64
  %38 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %37
  %39 = load float addrspace(3)* %38, align 4
  %40 = add nsw i32 %s.03, 1
  %41 = and i32 %40, 4095
  %42 = zext i32 %41 to i64
  %43 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %42
  %44 = load float addrspace(3)* %43, align 4
  %45 = add nsw i32 %s.03, 2
  %46 = and i32 %45, 4095
  %47 = zext i32 %46 to i64
  %48 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %47
  %49 = load float addrspace(3)* %48, align 4
  %50 = add nsw i32 %s.03, 3
  %51 = and i32 %50, 4095
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %52
  %54 = load float addrspace(3)* %53, align 4
  %55 = add nsw i32 %s.03, 4
  %56 = and i32 %55, 4095
  %57 = zext i32 %56 to i64
  %58 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %57
  %59 = load float addrspace(3)* %58, align 4
  %60 = add nsw i32 %s.03, 5
  %61 = and i32 %60, 4095
  %62 = zext i32 %61 to i64
  %63 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %62
  %64 = load float addrspace(3)* %63, align 4
  %65 = add nsw i32 %s.03, 6
  %66 = and i32 %65, 4095
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %67
  %69 = load float addrspace(3)* %68, align 4
  %70 = add nsw i32 %s.03, 7
  %71 = and i32 %70, 4095
  %72 = zext i32 %71 to i64
  %73 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %72
  %74 = load float addrspace(3)* %73, align 4
  %75 = add nsw i32 %s.03, 8
  %76 = and i32 %75, 4095
  %77 = zext i32 %76 to i64
  %78 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %77
  %79 = load float addrspace(3)* %78, align 4
  %80 = add nsw i32 %s.03, 9
  %81 = and i32 %80, 4095
  %82 = zext i32 %81 to i64
  %83 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %82
  %84 = load float addrspace(3)* %83, align 4
  %85 = add nsw i32 %s.03, 10
  %86 = and i32 %85, 4095
  %87 = zext i32 %86 to i64
  %88 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %87
  %89 = load float addrspace(3)* %88, align 4
  %90 = add nsw i32 %s.03, 11
  %91 = and i32 %90, 4095
  %92 = zext i32 %91 to i64
  %93 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %92
  %94 = load float addrspace(3)* %93, align 4
  %95 = add nsw i32 %s.03, 12
  %96 = and i32 %95, 4095
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %97
  %99 = load float addrspace(3)* %98, align 4
  %100 = add nsw i32 %s.03, 13
  %101 = and i32 %100, 4095
  %102 = zext i32 %101 to i64
  %103 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %102
  %104 = load float addrspace(3)* %103, align 4
  %105 = add nsw i32 %s.03, 14
  %106 = and i32 %105, 4095
  %107 = zext i32 %106 to i64
  %108 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %107
  %109 = load float addrspace(3)* %108, align 4
  %110 = add nsw i32 %s.03, 15
  %111 = and i32 %110, 4095
  %112 = zext i32 %111 to i64
  %113 = getelementptr inbounds [4096 x float] addrspace(3)* %1, i64 0, i64 %112
  %114 = load float addrspace(3)* %113, align 4
  %115 = fadd float %39, %44
  %116 = fadd float %115, %49
  %117 = fadd float %116, %54
  %118 = fadd float %117, %59
  %119 = fadd float %118, %64
  %120 = fadd float %119, %69
  %121 = fadd float %120, %74
  %122 = fadd float %121, %79
  %123 = fadd float %122, %84
  %124 = fadd float %123, %89
  %125 = fadd float %124, %94
  %126 = fadd float %125, %99
  %127 = fadd float %126, %104
  %128 = fadd float %127, %109
  %129 = fadd float %128, %114
  %130 = fadd float %sum.02, %129
  %131 = add nsw i32 %j.21, 1
  %132 = add nsw i32 %s.03, 16
  %133 = and i32 %132, 4095
  %exitcond = icmp eq i32 %131, 3000
  br i1 %exitcond, label %._crit_edge, label %35

._crit_edge:                                      ; preds = %35
  %"&pSB[currWI].offset38" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType39 = bitcast i8* %"&pSB[currWI].offset38" to i64*
  %loadedValue = load i64* %CastToValueType39, align 8
  %sext = shl i64 %loadedValue, 32
  %134 = ashr i64 %sext, 32
  %135 = getelementptr inbounds float addrspace(1)* %output, i64 %134
  store float %130, float addrspace(1)* %135, align 4
  %check.WI.iter55 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter55, label %thenBB52, label %SyncBB50

thenBB52:                                         ; preds = %._crit_edge
  %"CurrWI++56" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride58" = add nuw i64 %CurrSBIndex..1, 128
  br label %SyncBB

SyncBB50:                                         ; preds = %._crit_edge
  ret void
}

define void @____Vectorized_.readLocalMemory_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
bb.nph9:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  %temp95 = insertelement <16 x i32> undef, i32 %size, i32 0
  %vector96 = shufflevector <16 x i32> %temp95, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB1506

SyncBB1506:                                       ; preds = %thenBB, %bb.nph9
  %CurrWI..0 = phi i64 [ 0, %bb.nph9 ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %bb.nph9 ], [ %"loadedCurrSB+Stride", %thenBB ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %"&(pSB[currWI].offset)13" = or i64 %CurrSBIndex..0, 16
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)13"
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %5, i64* %CastToValueType, align 8
  %6 = load i64* %pWGId, align 8
  %7 = trunc i64 %6 to i32
  %8 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %9 = load i64* %8, align 8
  %broadcast191 = insertelement <16 x i64> undef, i64 %9, i32 0
  %broadcast292 = shufflevector <16 x i64> %broadcast191, <16 x i64> undef, <16 x i32> zeroinitializer
  %10 = add <16 x i64> %broadcast292, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %11 = trunc <16 x i64> %10 to <16 x i32>
  %"&(pSB[currWI].offset)149814" = or i64 %CurrSBIndex..0, 64
  %"&pSB[currWI].offset1499" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)149814"
  %CastToValueType1500 = bitcast i8* %"&pSB[currWI].offset1499" to <16 x i32>*
  store <16 x i32> %11, <16 x i32>* %CastToValueType1500, align 64
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %13 = load i64* %12, align 8
  %14 = trunc i64 %13 to i32
  %15 = icmp eq i32 %14, 0
  %16 = select i1 %15, i32 1, i32 %14
  %17 = sdiv i32 4096, %16
  %temp = insertelement <16 x i32> undef, i32 %17, i32 0
  %vector = shufflevector <16 x i32> %temp, <16 x i32> undef, <16 x i32> zeroinitializer
  %18 = mul nsw i32 %14, %7
  %temp93 = insertelement <16 x i32> undef, i32 %18, i32 0
  %vector94 = shufflevector <16 x i32> %temp93, <16 x i32> undef, <16 x i32> zeroinitializer
  %19 = mul nsw <16 x i32> %vector, %11
  %20 = add nsw <16 x i32> %vector94, %19
  %21 = sub nsw <16 x i32> %vector96, %20
  %22 = icmp sgt i32 %17, 0
  %temp97 = insertelement <16 x i1> undef, i1 %22, i32 0
  %vector98 = shufflevector <16 x i1> %temp97, <16 x i1> undef, <16 x i32> zeroinitializer
  %23 = icmp sgt <16 x i32> %21, zeroinitializer
  %or.cond799 = and <16 x i1> %vector98, %23
  %tmp22 = sub i32 0, %17
  %temp107 = insertelement <16 x i32> undef, i32 %tmp22, i32 0
  %vector108 = shufflevector <16 x i32> %temp107, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp28105 = add <16 x i32> %19, %vector94
  %tmp29106 = sub <16 x i32> %tmp28105, %vector96
  %tmp30 = icmp ult <16 x i32> %tmp29106, %vector108
  %umax109 = select <16 x i1> %tmp30, <16 x i32> %vector108, <16 x i32> %tmp29106
  %tmp31110 = sub <16 x i32> zeroinitializer, %umax109
  %ipred.i = bitcast <16 x i1> %or.cond799 to i16
  %val.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i, i16 %ipred.i) nounwind
  %tmp.i = and i32 %val.i, 1
  %res.i = icmp eq i32 %tmp.i, 0
  br i1 %res.i, label %.preheader1517, label %.critedge.preheader

.preheader1517:                                   ; preds = %SyncBB1506
  %negIncomingLoopMask111 = xor <16 x i1> %or.cond799, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %25

.critedge.preheader:                              ; preds = %postload1114, %SyncBB1506
  %merge103 = select <16 x i1> %or.cond799, <16 x i32> %tmp31110, <16 x i32> zeroinitializer
  %24 = icmp slt <16 x i32> %merge103, %vector
  %tmp = add i32 %17, -1
  %temp173 = insertelement <16 x i32> undef, i32 %tmp, i32 0
  %vector174 = shufflevector <16 x i32> %temp173, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp11175 = sub <16 x i32> %vector174, %merge103
  %tmp12176 = zext <16 x i32> %tmp11175 to <16 x i64>
  %tmp13177 = add <16 x i64> %tmp12176, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %tmp17178 = add <16 x i32> %merge103, %19
  %tmp18179 = zext <16 x i32> %tmp17178 to <16 x i64>
  %ipred.i1 = bitcast <16 x i1> %24 to i16
  %val.i2 = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1, i16 %ipred.i1) nounwind
  %tmp.i3 = and i32 %val.i2, 1
  %res.i4 = icmp eq i32 %tmp.i3, 0
  br i1 %res.i4, label %.critedge.preheader1518, label %bb.nph

.critedge.preheader1518:                          ; preds = %.critedge.preheader
  %negIncomingLoopMask40180 = xor <16 x i1> %24, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %.critedge

; <label>:25                                      ; preds = %postload1114, %.preheader1517
  %vectorPHI = phi <16 x i1> [ %loop_mask8170, %postload1114 ], [ %negIncomingLoopMask111, %.preheader1517 ]
  %vectorPHI113 = phi <16 x i1> [ %local_edge172, %postload1114 ], [ %or.cond799, %.preheader1517 ]
  %j.08 = phi i32 [ %122, %postload1114 ], [ 0, %.preheader1517 ]
  %extract133 = extractelement <16 x i1> %vectorPHI113, i32 0
  %extract134 = extractelement <16 x i1> %vectorPHI113, i32 1
  %extract135 = extractelement <16 x i1> %vectorPHI113, i32 2
  %extract136 = extractelement <16 x i1> %vectorPHI113, i32 3
  %extract137 = extractelement <16 x i1> %vectorPHI113, i32 4
  %extract138 = extractelement <16 x i1> %vectorPHI113, i32 5
  %extract139 = extractelement <16 x i1> %vectorPHI113, i32 6
  %extract140 = extractelement <16 x i1> %vectorPHI113, i32 7
  %extract141 = extractelement <16 x i1> %vectorPHI113, i32 8
  %extract142 = extractelement <16 x i1> %vectorPHI113, i32 9
  %extract143 = extractelement <16 x i1> %vectorPHI113, i32 10
  %extract144 = extractelement <16 x i1> %vectorPHI113, i32 11
  %extract145 = extractelement <16 x i1> %vectorPHI113, i32 12
  %extract146 = extractelement <16 x i1> %vectorPHI113, i32 13
  %extract147 = extractelement <16 x i1> %vectorPHI113, i32 14
  %extract148 = extractelement <16 x i1> %vectorPHI113, i32 15
  %temp114 = insertelement <16 x i32> undef, i32 %j.08, i32 0
  %vector115 = shufflevector <16 x i32> %temp114, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp33116 = add <16 x i32> %tmp28105, %vector115
  %tmp34117 = add <16 x i32> %19, %vector115
  %26 = extractelement <16 x i32> %tmp33116, i32 1
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds float addrspace(1)* %data, i64 %27
  %29 = extractelement <16 x i32> %tmp33116, i32 2
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds float addrspace(1)* %data, i64 %30
  %32 = extractelement <16 x i32> %tmp33116, i32 3
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds float addrspace(1)* %data, i64 %33
  %35 = extractelement <16 x i32> %tmp33116, i32 4
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds float addrspace(1)* %data, i64 %36
  %38 = extractelement <16 x i32> %tmp33116, i32 5
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds float addrspace(1)* %data, i64 %39
  %41 = extractelement <16 x i32> %tmp33116, i32 6
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds float addrspace(1)* %data, i64 %42
  %44 = extractelement <16 x i32> %tmp33116, i32 7
  %45 = sext i32 %44 to i64
  %46 = getelementptr inbounds float addrspace(1)* %data, i64 %45
  %47 = extractelement <16 x i32> %tmp33116, i32 8
  %48 = sext i32 %47 to i64
  %49 = getelementptr inbounds float addrspace(1)* %data, i64 %48
  %50 = extractelement <16 x i32> %tmp33116, i32 9
  %51 = sext i32 %50 to i64
  %52 = getelementptr inbounds float addrspace(1)* %data, i64 %51
  %53 = extractelement <16 x i32> %tmp33116, i32 10
  %54 = sext i32 %53 to i64
  %55 = getelementptr inbounds float addrspace(1)* %data, i64 %54
  %56 = extractelement <16 x i32> %tmp33116, i32 11
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float addrspace(1)* %data, i64 %57
  %59 = extractelement <16 x i32> %tmp33116, i32 12
  %60 = sext i32 %59 to i64
  %61 = getelementptr inbounds float addrspace(1)* %data, i64 %60
  %62 = extractelement <16 x i32> %tmp33116, i32 13
  %63 = sext i32 %62 to i64
  %64 = getelementptr inbounds float addrspace(1)* %data, i64 %63
  %65 = extractelement <16 x i32> %tmp33116, i32 14
  %66 = sext i32 %65 to i64
  %67 = getelementptr inbounds float addrspace(1)* %data, i64 %66
  %68 = extractelement <16 x i32> %tmp33116, i32 15
  %69 = sext i32 %68 to i64
  %70 = getelementptr inbounds float addrspace(1)* %data, i64 %69
  br i1 %extract133, label %preload1403, label %postload1404

preload1403:                                      ; preds = %25
  %71 = extractelement <16 x i32> %tmp33116, i32 0
  %72 = sext i32 %71 to i64
  %73 = getelementptr inbounds float addrspace(1)* %data, i64 %72
  %masked_load = load float addrspace(1)* %73, align 4
  br label %postload1404

postload1404:                                     ; preds = %preload1403, %25
  %phi1405 = phi float [ undef, %25 ], [ %masked_load, %preload1403 ]
  br i1 %extract134, label %preload1408, label %postload1409

preload1408:                                      ; preds = %postload1404
  %masked_load761 = load float addrspace(1)* %28, align 4
  br label %postload1409

postload1409:                                     ; preds = %preload1408, %postload1404
  %phi1410 = phi float [ undef, %postload1404 ], [ %masked_load761, %preload1408 ]
  br i1 %extract135, label %preload, label %postload

preload:                                          ; preds = %postload1409
  %masked_load762 = load float addrspace(1)* %31, align 4
  br label %postload

postload:                                         ; preds = %preload, %postload1409
  %phi = phi float [ undef, %postload1409 ], [ %masked_load762, %preload ]
  br i1 %extract136, label %preload1050, label %postload1051

preload1050:                                      ; preds = %postload
  %masked_load763 = load float addrspace(1)* %34, align 4
  br label %postload1051

postload1051:                                     ; preds = %preload1050, %postload
  %phi1052 = phi float [ undef, %postload ], [ %masked_load763, %preload1050 ]
  br i1 %extract137, label %preload1055, label %postload1056

preload1055:                                      ; preds = %postload1051
  %masked_load764 = load float addrspace(1)* %37, align 4
  br label %postload1056

postload1056:                                     ; preds = %preload1055, %postload1051
  %phi1057 = phi float [ undef, %postload1051 ], [ %masked_load764, %preload1055 ]
  br i1 %extract138, label %preload1060, label %postload1061

preload1060:                                      ; preds = %postload1056
  %masked_load765 = load float addrspace(1)* %40, align 4
  br label %postload1061

postload1061:                                     ; preds = %preload1060, %postload1056
  %phi1062 = phi float [ undef, %postload1056 ], [ %masked_load765, %preload1060 ]
  br i1 %extract139, label %preload1065, label %postload1066

preload1065:                                      ; preds = %postload1061
  %masked_load766 = load float addrspace(1)* %43, align 4
  br label %postload1066

postload1066:                                     ; preds = %preload1065, %postload1061
  %phi1067 = phi float [ undef, %postload1061 ], [ %masked_load766, %preload1065 ]
  br i1 %extract140, label %preload1070, label %postload1071

preload1070:                                      ; preds = %postload1066
  %masked_load767 = load float addrspace(1)* %46, align 4
  br label %postload1071

postload1071:                                     ; preds = %preload1070, %postload1066
  %phi1072 = phi float [ undef, %postload1066 ], [ %masked_load767, %preload1070 ]
  br i1 %extract141, label %preload1075, label %postload1076

preload1075:                                      ; preds = %postload1071
  %masked_load768 = load float addrspace(1)* %49, align 4
  br label %postload1076

postload1076:                                     ; preds = %preload1075, %postload1071
  %phi1077 = phi float [ undef, %postload1071 ], [ %masked_load768, %preload1075 ]
  br i1 %extract142, label %preload1080, label %postload1081

preload1080:                                      ; preds = %postload1076
  %masked_load769 = load float addrspace(1)* %52, align 4
  br label %postload1081

postload1081:                                     ; preds = %preload1080, %postload1076
  %phi1082 = phi float [ undef, %postload1076 ], [ %masked_load769, %preload1080 ]
  br i1 %extract143, label %preload1085, label %postload1086

preload1085:                                      ; preds = %postload1081
  %masked_load770 = load float addrspace(1)* %55, align 4
  br label %postload1086

postload1086:                                     ; preds = %preload1085, %postload1081
  %phi1087 = phi float [ undef, %postload1081 ], [ %masked_load770, %preload1085 ]
  br i1 %extract144, label %preload1090, label %postload1091

preload1090:                                      ; preds = %postload1086
  %masked_load771 = load float addrspace(1)* %58, align 4
  br label %postload1091

postload1091:                                     ; preds = %preload1090, %postload1086
  %phi1092 = phi float [ undef, %postload1086 ], [ %masked_load771, %preload1090 ]
  br i1 %extract145, label %preload1095, label %postload1096

preload1095:                                      ; preds = %postload1091
  %masked_load772 = load float addrspace(1)* %61, align 4
  br label %postload1096

postload1096:                                     ; preds = %preload1095, %postload1091
  %phi1097 = phi float [ undef, %postload1091 ], [ %masked_load772, %preload1095 ]
  br i1 %extract146, label %preload1100, label %postload1101

preload1100:                                      ; preds = %postload1096
  %masked_load773 = load float addrspace(1)* %64, align 4
  br label %postload1101

postload1101:                                     ; preds = %preload1100, %postload1096
  %phi1102 = phi float [ undef, %postload1096 ], [ %masked_load773, %preload1100 ]
  br i1 %extract147, label %preload1105, label %postload1106

preload1105:                                      ; preds = %postload1101
  %masked_load774 = load float addrspace(1)* %67, align 4
  br label %postload1106

postload1106:                                     ; preds = %preload1105, %postload1101
  %phi1107 = phi float [ undef, %postload1101 ], [ %masked_load774, %preload1105 ]
  br i1 %extract148, label %preload1110, label %postload1111

preload1110:                                      ; preds = %postload1106
  %masked_load775 = load float addrspace(1)* %70, align 4
  br label %postload1111

postload1111:                                     ; preds = %preload1110, %postload1106
  %phi1112 = phi float [ undef, %postload1106 ], [ %masked_load775, %preload1110 ]
  %74 = extractelement <16 x i32> %tmp34117, i32 1
  %75 = sext i32 %74 to i64
  %76 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %75
  %77 = extractelement <16 x i32> %tmp34117, i32 2
  %78 = sext i32 %77 to i64
  %79 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %78
  %80 = extractelement <16 x i32> %tmp34117, i32 3
  %81 = sext i32 %80 to i64
  %82 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %81
  %83 = extractelement <16 x i32> %tmp34117, i32 4
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %84
  %86 = extractelement <16 x i32> %tmp34117, i32 5
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %87
  %89 = extractelement <16 x i32> %tmp34117, i32 6
  %90 = sext i32 %89 to i64
  %91 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %90
  %92 = extractelement <16 x i32> %tmp34117, i32 7
  %93 = sext i32 %92 to i64
  %94 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %93
  %95 = extractelement <16 x i32> %tmp34117, i32 8
  %96 = sext i32 %95 to i64
  %97 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %96
  %98 = extractelement <16 x i32> %tmp34117, i32 9
  %99 = sext i32 %98 to i64
  %100 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %99
  %101 = extractelement <16 x i32> %tmp34117, i32 10
  %102 = sext i32 %101 to i64
  %103 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %102
  %104 = extractelement <16 x i32> %tmp34117, i32 11
  %105 = sext i32 %104 to i64
  %106 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %105
  %107 = extractelement <16 x i32> %tmp34117, i32 12
  %108 = sext i32 %107 to i64
  %109 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %108
  %110 = extractelement <16 x i32> %tmp34117, i32 13
  %111 = sext i32 %110 to i64
  %112 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %111
  %113 = extractelement <16 x i32> %tmp34117, i32 14
  %114 = sext i32 %113 to i64
  %115 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %114
  %116 = extractelement <16 x i32> %tmp34117, i32 15
  %117 = sext i32 %116 to i64
  %118 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %117
  br i1 %extract133, label %preload1406, label %postload1407

preload1406:                                      ; preds = %postload1111
  %119 = extractelement <16 x i32> %tmp34117, i32 0
  %120 = sext i32 %119 to i64
  %121 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %120
  store float %phi1405, float addrspace(3)* %121, align 4
  br label %postload1407

postload1407:                                     ; preds = %preload1406, %postload1111
  br i1 %extract134, label %preload1411, label %postload1412

preload1411:                                      ; preds = %postload1407
  store float %phi1410, float addrspace(3)* %76, align 4
  br label %postload1412

postload1412:                                     ; preds = %preload1411, %postload1407
  br i1 %extract135, label %preload1048, label %postload1049

preload1048:                                      ; preds = %postload1412
  store float %phi, float addrspace(3)* %79, align 4
  br label %postload1049

postload1049:                                     ; preds = %preload1048, %postload1412
  br i1 %extract136, label %preload1053, label %postload1054

preload1053:                                      ; preds = %postload1049
  store float %phi1052, float addrspace(3)* %82, align 4
  br label %postload1054

postload1054:                                     ; preds = %preload1053, %postload1049
  br i1 %extract137, label %preload1058, label %postload1059

preload1058:                                      ; preds = %postload1054
  store float %phi1057, float addrspace(3)* %85, align 4
  br label %postload1059

postload1059:                                     ; preds = %preload1058, %postload1054
  br i1 %extract138, label %preload1063, label %postload1064

preload1063:                                      ; preds = %postload1059
  store float %phi1062, float addrspace(3)* %88, align 4
  br label %postload1064

postload1064:                                     ; preds = %preload1063, %postload1059
  br i1 %extract139, label %preload1068, label %postload1069

preload1068:                                      ; preds = %postload1064
  store float %phi1067, float addrspace(3)* %91, align 4
  br label %postload1069

postload1069:                                     ; preds = %preload1068, %postload1064
  br i1 %extract140, label %preload1073, label %postload1074

preload1073:                                      ; preds = %postload1069
  store float %phi1072, float addrspace(3)* %94, align 4
  br label %postload1074

postload1074:                                     ; preds = %preload1073, %postload1069
  br i1 %extract141, label %preload1078, label %postload1079

preload1078:                                      ; preds = %postload1074
  store float %phi1077, float addrspace(3)* %97, align 4
  br label %postload1079

postload1079:                                     ; preds = %preload1078, %postload1074
  br i1 %extract142, label %preload1083, label %postload1084

preload1083:                                      ; preds = %postload1079
  store float %phi1082, float addrspace(3)* %100, align 4
  br label %postload1084

postload1084:                                     ; preds = %preload1083, %postload1079
  br i1 %extract143, label %preload1088, label %postload1089

preload1088:                                      ; preds = %postload1084
  store float %phi1087, float addrspace(3)* %103, align 4
  br label %postload1089

postload1089:                                     ; preds = %preload1088, %postload1084
  br i1 %extract144, label %preload1093, label %postload1094

preload1093:                                      ; preds = %postload1089
  store float %phi1092, float addrspace(3)* %106, align 4
  br label %postload1094

postload1094:                                     ; preds = %preload1093, %postload1089
  br i1 %extract145, label %preload1098, label %postload1099

preload1098:                                      ; preds = %postload1094
  store float %phi1097, float addrspace(3)* %109, align 4
  br label %postload1099

postload1099:                                     ; preds = %preload1098, %postload1094
  br i1 %extract146, label %preload1103, label %postload1104

preload1103:                                      ; preds = %postload1099
  store float %phi1102, float addrspace(3)* %112, align 4
  br label %postload1104

postload1104:                                     ; preds = %preload1103, %postload1099
  br i1 %extract147, label %preload1108, label %postload1109

preload1108:                                      ; preds = %postload1104
  store float %phi1107, float addrspace(3)* %115, align 4
  br label %postload1109

postload1109:                                     ; preds = %preload1108, %postload1104
  br i1 %extract148, label %preload1113, label %postload1114

preload1113:                                      ; preds = %postload1109
  store float %phi1112, float addrspace(3)* %118, align 4
  br label %postload1114

postload1114:                                     ; preds = %preload1113, %postload1109
  %122 = add nsw i32 %j.08, 1
  %temp165 = insertelement <16 x i32> undef, i32 %122, i32 0
  %vector166 = shufflevector <16 x i32> %temp165, <16 x i32> undef, <16 x i32> zeroinitializer
  %exitcond32 = icmp eq <16 x i32> %vector166, %tmp31110
  %notCond167 = xor <16 x i1> %exitcond32, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr168 = and <16 x i1> %vectorPHI113, %exitcond32
  %loop_mask8170 = or <16 x i1> %vectorPHI, %who_left_tr168
  %curr_loop_mask171 = or <16 x i1> %loop_mask8170, %who_left_tr168
  %ipred.i5 = bitcast <16 x i1> %curr_loop_mask171 to i16
  %val.i6 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5, i16 %ipred.i5) nounwind
  %tmp.i7 = and i32 %val.i6, 1
  %res.i8 = icmp eq i32 %tmp.i7, 0
  %local_edge172 = and <16 x i1> %vectorPHI113, %notCond167
  br i1 %res.i8, label %25, label %.critedge.preheader

.critedge:                                        ; preds = %postload1492, %.critedge.preheader1518
  %vectorPHI181 = phi <16 x i1> [ %loop_mask15225, %postload1492 ], [ %negIncomingLoopMask40180, %.critedge.preheader1518 ]
  %vectorPHI183 = phi <16 x i1> [ %local_edge20227, %postload1492 ], [ %24, %.critedge.preheader1518 ]
  %indvar = phi i64 [ %indvar.next, %postload1492 ], [ 0, %.critedge.preheader1518 ]
  %extract187 = extractelement <16 x i1> %vectorPHI183, i32 0
  %extract188 = extractelement <16 x i1> %vectorPHI183, i32 1
  %extract189 = extractelement <16 x i1> %vectorPHI183, i32 2
  %extract190 = extractelement <16 x i1> %vectorPHI183, i32 3
  %extract191 = extractelement <16 x i1> %vectorPHI183, i32 4
  %extract192 = extractelement <16 x i1> %vectorPHI183, i32 5
  %extract193 = extractelement <16 x i1> %vectorPHI183, i32 6
  %extract194 = extractelement <16 x i1> %vectorPHI183, i32 7
  %extract195 = extractelement <16 x i1> %vectorPHI183, i32 8
  %extract196 = extractelement <16 x i1> %vectorPHI183, i32 9
  %extract197 = extractelement <16 x i1> %vectorPHI183, i32 10
  %extract198 = extractelement <16 x i1> %vectorPHI183, i32 11
  %extract199 = extractelement <16 x i1> %vectorPHI183, i32 12
  %extract200 = extractelement <16 x i1> %vectorPHI183, i32 13
  %extract201 = extractelement <16 x i1> %vectorPHI183, i32 14
  %extract202 = extractelement <16 x i1> %vectorPHI183, i32 15
  %temp184 = insertelement <16 x i64> undef, i64 %indvar, i32 0
  %vector185 = shufflevector <16 x i64> %temp184, <16 x i64> undef, <16 x i32> zeroinitializer
  %scevgep = getelementptr float addrspace(1)* %data, i64 %indvar
  %tmp19186 = add <16 x i64> %tmp18179, %vector185
  br i1 %extract187, label %preload1413, label %postload1414

preload1413:                                      ; preds = %.critedge
  %masked_load776 = load float addrspace(1)* %scevgep, align 4
  br label %postload1414

postload1414:                                     ; preds = %preload1413, %.critedge
  %phi1415 = phi float [ undef, %.critedge ], [ %masked_load776, %preload1413 ]
  br i1 %extract188, label %preload1418, label %postload1419

preload1418:                                      ; preds = %postload1414
  %masked_load777 = load float addrspace(1)* %scevgep, align 4
  br label %postload1419

postload1419:                                     ; preds = %preload1418, %postload1414
  %phi1420 = phi float [ undef, %postload1414 ], [ %masked_load777, %preload1418 ]
  br i1 %extract189, label %preload1423, label %postload1424

preload1423:                                      ; preds = %postload1419
  %masked_load778 = load float addrspace(1)* %scevgep, align 4
  br label %postload1424

postload1424:                                     ; preds = %preload1423, %postload1419
  %phi1425 = phi float [ undef, %postload1419 ], [ %masked_load778, %preload1423 ]
  br i1 %extract190, label %preload1428, label %postload1429

preload1428:                                      ; preds = %postload1424
  %masked_load779 = load float addrspace(1)* %scevgep, align 4
  br label %postload1429

postload1429:                                     ; preds = %preload1428, %postload1424
  %phi1430 = phi float [ undef, %postload1424 ], [ %masked_load779, %preload1428 ]
  br i1 %extract191, label %preload1433, label %postload1434

preload1433:                                      ; preds = %postload1429
  %masked_load780 = load float addrspace(1)* %scevgep, align 4
  br label %postload1434

postload1434:                                     ; preds = %preload1433, %postload1429
  %phi1435 = phi float [ undef, %postload1429 ], [ %masked_load780, %preload1433 ]
  br i1 %extract192, label %preload1438, label %postload1439

preload1438:                                      ; preds = %postload1434
  %masked_load781 = load float addrspace(1)* %scevgep, align 4
  br label %postload1439

postload1439:                                     ; preds = %preload1438, %postload1434
  %phi1440 = phi float [ undef, %postload1434 ], [ %masked_load781, %preload1438 ]
  br i1 %extract193, label %preload1443, label %postload1444

preload1443:                                      ; preds = %postload1439
  %masked_load782 = load float addrspace(1)* %scevgep, align 4
  br label %postload1444

postload1444:                                     ; preds = %preload1443, %postload1439
  %phi1445 = phi float [ undef, %postload1439 ], [ %masked_load782, %preload1443 ]
  br i1 %extract194, label %preload1448, label %postload1449

preload1448:                                      ; preds = %postload1444
  %masked_load783 = load float addrspace(1)* %scevgep, align 4
  br label %postload1449

postload1449:                                     ; preds = %preload1448, %postload1444
  %phi1450 = phi float [ undef, %postload1444 ], [ %masked_load783, %preload1448 ]
  br i1 %extract195, label %preload1453, label %postload1454

preload1453:                                      ; preds = %postload1449
  %masked_load784 = load float addrspace(1)* %scevgep, align 4
  br label %postload1454

postload1454:                                     ; preds = %preload1453, %postload1449
  %phi1455 = phi float [ undef, %postload1449 ], [ %masked_load784, %preload1453 ]
  br i1 %extract196, label %preload1458, label %postload1459

preload1458:                                      ; preds = %postload1454
  %masked_load785 = load float addrspace(1)* %scevgep, align 4
  br label %postload1459

postload1459:                                     ; preds = %preload1458, %postload1454
  %phi1460 = phi float [ undef, %postload1454 ], [ %masked_load785, %preload1458 ]
  br i1 %extract197, label %preload1463, label %postload1464

preload1463:                                      ; preds = %postload1459
  %masked_load786 = load float addrspace(1)* %scevgep, align 4
  br label %postload1464

postload1464:                                     ; preds = %preload1463, %postload1459
  %phi1465 = phi float [ undef, %postload1459 ], [ %masked_load786, %preload1463 ]
  br i1 %extract198, label %preload1468, label %postload1469

preload1468:                                      ; preds = %postload1464
  %masked_load787 = load float addrspace(1)* %scevgep, align 4
  br label %postload1469

postload1469:                                     ; preds = %preload1468, %postload1464
  %phi1470 = phi float [ undef, %postload1464 ], [ %masked_load787, %preload1468 ]
  br i1 %extract199, label %preload1473, label %postload1474

preload1473:                                      ; preds = %postload1469
  %masked_load788 = load float addrspace(1)* %scevgep, align 4
  br label %postload1474

postload1474:                                     ; preds = %preload1473, %postload1469
  %phi1475 = phi float [ undef, %postload1469 ], [ %masked_load788, %preload1473 ]
  br i1 %extract200, label %preload1478, label %postload1479

preload1478:                                      ; preds = %postload1474
  %masked_load789 = load float addrspace(1)* %scevgep, align 4
  br label %postload1479

postload1479:                                     ; preds = %preload1478, %postload1474
  %phi1480 = phi float [ undef, %postload1474 ], [ %masked_load789, %preload1478 ]
  br i1 %extract201, label %preload1483, label %postload1484

preload1483:                                      ; preds = %postload1479
  %masked_load790 = load float addrspace(1)* %scevgep, align 4
  br label %postload1484

postload1484:                                     ; preds = %preload1483, %postload1479
  %phi1485 = phi float [ undef, %postload1479 ], [ %masked_load790, %preload1483 ]
  br i1 %extract202, label %preload1488, label %postload1489

preload1488:                                      ; preds = %postload1484
  %masked_load791 = load float addrspace(1)* %scevgep, align 4
  br label %postload1489

postload1489:                                     ; preds = %preload1488, %postload1484
  %phi1490 = phi float [ undef, %postload1484 ], [ %masked_load791, %preload1488 ]
  %sext35203 = shl <16 x i64> %tmp19186, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %123 = ashr <16 x i64> %sext35203, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract205 = extractelement <16 x i64> %123, i32 1
  %extract206 = extractelement <16 x i64> %123, i32 2
  %extract207 = extractelement <16 x i64> %123, i32 3
  %extract208 = extractelement <16 x i64> %123, i32 4
  %extract209 = extractelement <16 x i64> %123, i32 5
  %extract210 = extractelement <16 x i64> %123, i32 6
  %extract211 = extractelement <16 x i64> %123, i32 7
  %extract212 = extractelement <16 x i64> %123, i32 8
  %extract213 = extractelement <16 x i64> %123, i32 9
  %extract214 = extractelement <16 x i64> %123, i32 10
  %extract215 = extractelement <16 x i64> %123, i32 11
  %extract216 = extractelement <16 x i64> %123, i32 12
  %extract217 = extractelement <16 x i64> %123, i32 13
  %extract218 = extractelement <16 x i64> %123, i32 14
  %extract219 = extractelement <16 x i64> %123, i32 15
  %124 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract205
  %125 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract206
  %126 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract207
  %127 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract208
  %128 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract209
  %129 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract210
  %130 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract211
  %131 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract212
  %132 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract213
  %133 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract214
  %134 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract215
  %135 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract216
  %136 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract217
  %137 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract218
  %138 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract219
  br i1 %extract187, label %preload1416, label %postload1417

preload1416:                                      ; preds = %postload1489
  %extract204 = extractelement <16 x i64> %123, i32 0
  %139 = getelementptr inbounds [4096 x float] addrspace(3)* %0, i64 0, i64 %extract204
  store float %phi1415, float addrspace(3)* %139, align 4
  br label %postload1417

postload1417:                                     ; preds = %preload1416, %postload1489
  br i1 %extract188, label %preload1421, label %postload1422

preload1421:                                      ; preds = %postload1417
  store float %phi1420, float addrspace(3)* %124, align 4
  br label %postload1422

postload1422:                                     ; preds = %preload1421, %postload1417
  br i1 %extract189, label %preload1426, label %postload1427

preload1426:                                      ; preds = %postload1422
  store float %phi1425, float addrspace(3)* %125, align 4
  br label %postload1427

postload1427:                                     ; preds = %preload1426, %postload1422
  br i1 %extract190, label %preload1431, label %postload1432

preload1431:                                      ; preds = %postload1427
  store float %phi1430, float addrspace(3)* %126, align 4
  br label %postload1432

postload1432:                                     ; preds = %preload1431, %postload1427
  br i1 %extract191, label %preload1436, label %postload1437

preload1436:                                      ; preds = %postload1432
  store float %phi1435, float addrspace(3)* %127, align 4
  br label %postload1437

postload1437:                                     ; preds = %preload1436, %postload1432
  br i1 %extract192, label %preload1441, label %postload1442

preload1441:                                      ; preds = %postload1437
  store float %phi1440, float addrspace(3)* %128, align 4
  br label %postload1442

postload1442:                                     ; preds = %preload1441, %postload1437
  br i1 %extract193, label %preload1446, label %postload1447

preload1446:                                      ; preds = %postload1442
  store float %phi1445, float addrspace(3)* %129, align 4
  br label %postload1447

postload1447:                                     ; preds = %preload1446, %postload1442
  br i1 %extract194, label %preload1451, label %postload1452

preload1451:                                      ; preds = %postload1447
  store float %phi1450, float addrspace(3)* %130, align 4
  br label %postload1452

postload1452:                                     ; preds = %preload1451, %postload1447
  br i1 %extract195, label %preload1456, label %postload1457

preload1456:                                      ; preds = %postload1452
  store float %phi1455, float addrspace(3)* %131, align 4
  br label %postload1457

postload1457:                                     ; preds = %preload1456, %postload1452
  br i1 %extract196, label %preload1461, label %postload1462

preload1461:                                      ; preds = %postload1457
  store float %phi1460, float addrspace(3)* %132, align 4
  br label %postload1462

postload1462:                                     ; preds = %preload1461, %postload1457
  br i1 %extract197, label %preload1466, label %postload1467

preload1466:                                      ; preds = %postload1462
  store float %phi1465, float addrspace(3)* %133, align 4
  br label %postload1467

postload1467:                                     ; preds = %preload1466, %postload1462
  br i1 %extract198, label %preload1471, label %postload1472

preload1471:                                      ; preds = %postload1467
  store float %phi1470, float addrspace(3)* %134, align 4
  br label %postload1472

postload1472:                                     ; preds = %preload1471, %postload1467
  br i1 %extract199, label %preload1476, label %postload1477

preload1476:                                      ; preds = %postload1472
  store float %phi1475, float addrspace(3)* %135, align 4
  br label %postload1477

postload1477:                                     ; preds = %preload1476, %postload1472
  br i1 %extract200, label %preload1481, label %postload1482

preload1481:                                      ; preds = %postload1477
  store float %phi1480, float addrspace(3)* %136, align 4
  br label %postload1482

postload1482:                                     ; preds = %preload1481, %postload1477
  br i1 %extract201, label %preload1486, label %postload1487

preload1486:                                      ; preds = %postload1482
  store float %phi1485, float addrspace(3)* %137, align 4
  br label %postload1487

postload1487:                                     ; preds = %preload1486, %postload1482
  br i1 %extract202, label %preload1491, label %postload1492

preload1491:                                      ; preds = %postload1487
  store float %phi1490, float addrspace(3)* %138, align 4
  br label %postload1492

postload1492:                                     ; preds = %preload1491, %postload1487
  %indvar.next = add i64 %indvar, 1
  %temp220 = insertelement <16 x i64> undef, i64 %indvar.next, i32 0
  %vector221 = shufflevector <16 x i64> %temp220, <16 x i64> undef, <16 x i32> zeroinitializer
  %exitcond14 = icmp eq <16 x i64> %vector221, %tmp13177
  %notCond11222 = xor <16 x i1> %exitcond14, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr12223 = and <16 x i1> %vectorPHI183, %exitcond14
  %loop_mask15225 = or <16 x i1> %vectorPHI181, %who_left_tr12223
  %curr_loop_mask17226 = or <16 x i1> %loop_mask15225, %who_left_tr12223
  %ipred.i9 = bitcast <16 x i1> %curr_loop_mask17226 to i16
  %val.i10 = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9, i16 %ipred.i9) nounwind
  %tmp.i11 = and i32 %val.i10, 1
  %res.i12 = icmp eq i32 %tmp.i11, 0
  %local_edge20227 = and <16 x i1> %vectorPHI183, %notCond11222
  br i1 %res.i12, label %.critedge, label %bb.nph

bb.nph:                                           ; preds = %postload1492, %.critedge.preheader
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %"Barrier BB"

thenBB:                                           ; preds = %bb.nph
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 128
  br label %SyncBB1506

"Barrier BB":                                     ; preds = %bb.nph, %thenBB1509
  %CurrWI..1 = phi i64 [ %"CurrWI++1513", %thenBB1509 ], [ 0, %bb.nph ]
  %check.WI.iter1512 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter1512, label %thenBB1509, label %SyncBB1507

thenBB1509:                                       ; preds = %"Barrier BB"
  %"CurrWI++1513" = add nuw i64 %CurrWI..1, 1
  br label %"Barrier BB"

SyncBB1507:                                       ; preds = %"Barrier BB"
  ret void
}

declare i32 @llvm.x86.mic.kortestz(i16, i16) nounwind readnone

declare i32 @llvm.x86.mic.kortestc(i16, i16) nounwind readnone

define void @readLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i32*
  %7 = load i32* %6, align 4
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to i8 addrspace(3)**
  %10 = load i8 addrspace(3)** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 32
  %12 = bitcast i8* %11 to %struct.WorkDim**
  %13 = load %struct.WorkDim** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 40
  %15 = bitcast i8* %14 to i64**
  %16 = load i64** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 48
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 56
  %21 = bitcast i8* %20 to %struct.PaddedDimId**
  %22 = load %struct.PaddedDimId** %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 72
  %24 = bitcast i8* %23 to i64*
  %25 = load i64* %24, align 8
  %26 = getelementptr i8* %pBuffer, i64 80
  %27 = bitcast i8* %26 to i8**
  %28 = load i8** %27, align 8
  %29 = bitcast i8 addrspace(3)* %10 to [4096 x float] addrspace(3)*
  br label %SyncBB49.i

SyncBB49.i:                                       ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %30 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %31 = load i64* %30, align 8
  %32 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = add i64 %31, %33
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %34, i64* %CastToValueType.i, align 8
  %35 = load i64* %16, align 8
  %36 = trunc i64 %35 to i32
  %37 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..0.i, i32 0, i64 0
  %38 = load i64* %37, align 8
  %39 = trunc i64 %38 to i32
  %"&(pSB[currWI].offset)411.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset42.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)411.i"
  %CastToValueType43.i = bitcast i8* %"&pSB[currWI].offset42.i" to i32*
  store i32 %39, i32* %CastToValueType43.i, align 4
  %40 = getelementptr %struct.WorkDim* %13, i64 0, i32 3, i64 0
  %41 = load i64* %40, align 8
  %42 = trunc i64 %41 to i32
  %43 = icmp eq i32 %42, 0
  %44 = select i1 %43, i32 1, i32 %42
  %45 = sdiv i32 4096, %44
  %46 = mul nsw i32 %42, %36
  %47 = mul nsw i32 %45, %39
  %48 = add nsw i32 %46, %47
  %49 = sub nsw i32 %7, %48
  %50 = icmp sgt i32 %45, 0
  %51 = icmp sgt i32 %49, 0
  %or.cond7.i = and i1 %50, %51
  br i1 %or.cond7.i, label %bb.nph9.i, label %.critedge.preheader.i

.critedge.preheader.i:                            ; preds = %53, %SyncBB49.i
  %j.0.lcssa.i = phi i32 [ 0, %SyncBB49.i ], [ %tmp31.i, %53 ]
  %52 = icmp slt i32 %j.0.lcssa.i, %45
  br i1 %52, label %bb.nph6.i, label %bb.nph.i

bb.nph9.i:                                        ; preds = %SyncBB49.i
  %tmp22.i = sub i32 0, %45
  %tmp28.i = add i32 %47, %46
  %tmp29.i = sub i32 %tmp28.i, %7
  %tmp30.i = icmp ult i32 %tmp29.i, %tmp22.i
  %umax.i = select i1 %tmp30.i, i32 %tmp22.i, i32 %tmp29.i
  %tmp31.i = sub i32 0, %umax.i
  br label %53

; <label>:53                                      ; preds = %53, %bb.nph9.i
  %j.08.i = phi i32 [ 0, %bb.nph9.i ], [ %59, %53 ]
  %tmp33.i = add i32 %tmp28.i, %j.08.i
  %tmp34.i = add i32 %47, %j.08.i
  %54 = sext i32 %tmp33.i to i64
  %55 = getelementptr inbounds float addrspace(1)* %1, i64 %54
  %56 = load float addrspace(1)* %55, align 4
  %57 = sext i32 %tmp34.i to i64
  %58 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %57
  store float %56, float addrspace(3)* %58, align 4
  %59 = add nsw i32 %j.08.i, 1
  %exitcond32.i = icmp eq i32 %59, %tmp31.i
  br i1 %exitcond32.i, label %.critedge.preheader.i, label %53

bb.nph6.i:                                        ; preds = %.critedge.preheader.i
  %tmp.i = add i32 %45, -1
  %tmp11.i = sub i32 %tmp.i, %j.0.lcssa.i
  %tmp12.i = zext i32 %tmp11.i to i64
  %tmp13.i = add i64 %tmp12.i, 1
  %tmp17.i = add i32 %j.0.lcssa.i, %47
  %tmp18.i = zext i32 %tmp17.i to i64
  br label %.critedge.i

.critedge.i:                                      ; preds = %.critedge.i, %bb.nph6.i
  %indvar.i = phi i64 [ 0, %bb.nph6.i ], [ %indvar.next.i, %.critedge.i ]
  %scevgep.i = getelementptr float addrspace(1)* %1, i64 %indvar.i
  %tmp19.i = add i64 %tmp18.i, %indvar.i
  %60 = load float addrspace(1)* %scevgep.i, align 4
  %sext35.i = shl i64 %tmp19.i, 32
  %61 = ashr i64 %sext35.i, 32
  %62 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %61
  store float %60, float addrspace(3)* %62, align 4
  %indvar.next.i = add i64 %indvar.i, 1
  %exitcond14.i = icmp eq i64 %indvar.next.i, %tmp13.i
  br i1 %exitcond14.i, label %bb.nph.i, label %.critedge.i

bb.nph.i:                                         ; preds = %.critedge.i, %.critedge.preheader.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %bb.nph.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB49.i

SyncBB.i:                                         ; preds = %thenBB52.i, %bb.nph.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++56.i", %thenBB52.i ], [ 0, %bb.nph.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride58.i", %thenBB52.i ], [ 0, %bb.nph.i ]
  %"&(pSB[currWI].offset)452.i" = or i64 %CurrSBIndex..1.i, 8
  %"&pSB[currWI].offset46.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)452.i"
  %CastToValueType47.i = bitcast i8* %"&pSB[currWI].offset46.i" to i32*
  %loadedValue48.i = load i32* %CastToValueType47.i, align 4
  br label %63

; <label>:63                                      ; preds = %63, %SyncBB.i
  %s.03.i = phi i32 [ %loadedValue48.i, %SyncBB.i ], [ %161, %63 ]
  %sum.02.i = phi float [ 0.000000e+00, %SyncBB.i ], [ %158, %63 ]
  %j.21.i = phi i32 [ 0, %SyncBB.i ], [ %159, %63 ]
  %64 = and i32 %s.03.i, 4095
  %65 = zext i32 %64 to i64
  %66 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %65
  %67 = load float addrspace(3)* %66, align 4
  %68 = add nsw i32 %s.03.i, 1
  %69 = and i32 %68, 4095
  %70 = zext i32 %69 to i64
  %71 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %70
  %72 = load float addrspace(3)* %71, align 4
  %73 = add nsw i32 %s.03.i, 2
  %74 = and i32 %73, 4095
  %75 = zext i32 %74 to i64
  %76 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %75
  %77 = load float addrspace(3)* %76, align 4
  %78 = add nsw i32 %s.03.i, 3
  %79 = and i32 %78, 4095
  %80 = zext i32 %79 to i64
  %81 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %80
  %82 = load float addrspace(3)* %81, align 4
  %83 = add nsw i32 %s.03.i, 4
  %84 = and i32 %83, 4095
  %85 = zext i32 %84 to i64
  %86 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %85
  %87 = load float addrspace(3)* %86, align 4
  %88 = add nsw i32 %s.03.i, 5
  %89 = and i32 %88, 4095
  %90 = zext i32 %89 to i64
  %91 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %90
  %92 = load float addrspace(3)* %91, align 4
  %93 = add nsw i32 %s.03.i, 6
  %94 = and i32 %93, 4095
  %95 = zext i32 %94 to i64
  %96 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %95
  %97 = load float addrspace(3)* %96, align 4
  %98 = add nsw i32 %s.03.i, 7
  %99 = and i32 %98, 4095
  %100 = zext i32 %99 to i64
  %101 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %100
  %102 = load float addrspace(3)* %101, align 4
  %103 = add nsw i32 %s.03.i, 8
  %104 = and i32 %103, 4095
  %105 = zext i32 %104 to i64
  %106 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %105
  %107 = load float addrspace(3)* %106, align 4
  %108 = add nsw i32 %s.03.i, 9
  %109 = and i32 %108, 4095
  %110 = zext i32 %109 to i64
  %111 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %110
  %112 = load float addrspace(3)* %111, align 4
  %113 = add nsw i32 %s.03.i, 10
  %114 = and i32 %113, 4095
  %115 = zext i32 %114 to i64
  %116 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %115
  %117 = load float addrspace(3)* %116, align 4
  %118 = add nsw i32 %s.03.i, 11
  %119 = and i32 %118, 4095
  %120 = zext i32 %119 to i64
  %121 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %120
  %122 = load float addrspace(3)* %121, align 4
  %123 = add nsw i32 %s.03.i, 12
  %124 = and i32 %123, 4095
  %125 = zext i32 %124 to i64
  %126 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %125
  %127 = load float addrspace(3)* %126, align 4
  %128 = add nsw i32 %s.03.i, 13
  %129 = and i32 %128, 4095
  %130 = zext i32 %129 to i64
  %131 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %130
  %132 = load float addrspace(3)* %131, align 4
  %133 = add nsw i32 %s.03.i, 14
  %134 = and i32 %133, 4095
  %135 = zext i32 %134 to i64
  %136 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %135
  %137 = load float addrspace(3)* %136, align 4
  %138 = add nsw i32 %s.03.i, 15
  %139 = and i32 %138, 4095
  %140 = zext i32 %139 to i64
  %141 = getelementptr inbounds [4096 x float] addrspace(3)* %29, i64 0, i64 %140
  %142 = load float addrspace(3)* %141, align 4
  %143 = fadd float %67, %72
  %144 = fadd float %143, %77
  %145 = fadd float %144, %82
  %146 = fadd float %145, %87
  %147 = fadd float %146, %92
  %148 = fadd float %147, %97
  %149 = fadd float %148, %102
  %150 = fadd float %149, %107
  %151 = fadd float %150, %112
  %152 = fadd float %151, %117
  %153 = fadd float %152, %122
  %154 = fadd float %153, %127
  %155 = fadd float %154, %132
  %156 = fadd float %155, %137
  %157 = fadd float %156, %142
  %158 = fadd float %sum.02.i, %157
  %159 = add nsw i32 %j.21.i, 1
  %160 = add nsw i32 %s.03.i, 16
  %161 = and i32 %160, 4095
  %exitcond.i = icmp eq i32 %159, 3000
  br i1 %exitcond.i, label %._crit_edge.i, label %63

._crit_edge.i:                                    ; preds = %63
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i64*
  %loadedValue.i = load i64* %CastToValueType39.i, align 8
  %sext.i = shl i64 %loadedValue.i, 32
  %162 = ashr i64 %sext.i, 32
  %163 = getelementptr inbounds float addrspace(1)* %4, i64 %162
  store float %158, float addrspace(1)* %163, align 4
  %check.WI.iter55.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter55.i, label %thenBB52.i, label %__readLocalMemory_separated_args.exit

thenBB52.i:                                       ; preds = %._crit_edge.i
  %"CurrWI++56.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride58.i" = add nuw i64 %CurrSBIndex..1.i, 128
  br label %SyncBB.i

__readLocalMemory_separated_args.exit:            ; preds = %._crit_edge.i
  ret void
}

define void @__Vectorized_.readLocalMemory(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to i32*
  %4 = load i32* %3, align 4
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to i8 addrspace(3)**
  %7 = load i8 addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to %struct.WorkDim**
  %10 = load %struct.WorkDim** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to i64**
  %13 = load i64** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 48
  %15 = bitcast i8* %14 to %struct.PaddedDimId**
  %16 = load %struct.PaddedDimId** %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 56
  %18 = bitcast i8* %17 to %struct.PaddedDimId**
  %19 = load %struct.PaddedDimId** %18, align 8
  %20 = getelementptr i8* %pBuffer, i64 72
  %21 = bitcast i8* %20 to i64*
  %22 = load i64* %21, align 8
  %23 = getelementptr i8* %pBuffer, i64 80
  %24 = bitcast i8* %23 to i8**
  %25 = load i8** %24, align 8
  %26 = bitcast i8 addrspace(3)* %7 to [4096 x float] addrspace(3)*
  %temp95.i = insertelement <16 x i32> undef, i32 %4, i32 0
  %vector96.i = shufflevector <16 x i32> %temp95.i, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %SyncBB1506.i

SyncBB1506.i:                                     ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  %27 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %28 = load i64* %27, align 8
  %29 = getelementptr %struct.PaddedDimId* %16, i64 0, i32 0, i64 0
  %30 = load i64* %29, align 8
  %31 = add i64 %28, %30
  %"&(pSB[currWI].offset)13.i" = or i64 %CurrSBIndex..0.i, 16
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)13.i"
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %31, i64* %CastToValueType.i, align 8
  %32 = load i64* %13, align 8
  %33 = trunc i64 %32 to i32
  %34 = getelementptr %struct.PaddedDimId* %19, i64 %CurrWI..0.i, i32 0, i64 0
  %35 = load i64* %34, align 8
  %broadcast191.i = insertelement <16 x i64> undef, i64 %35, i32 0
  %broadcast292.i = shufflevector <16 x i64> %broadcast191.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %36 = add <16 x i64> %broadcast292.i, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %37 = trunc <16 x i64> %36 to <16 x i32>
  %"&(pSB[currWI].offset)149814.i" = or i64 %CurrSBIndex..0.i, 64
  %"&pSB[currWI].offset1499.i" = getelementptr inbounds i8* %25, i64 %"&(pSB[currWI].offset)149814.i"
  %CastToValueType1500.i = bitcast i8* %"&pSB[currWI].offset1499.i" to <16 x i32>*
  store <16 x i32> %37, <16 x i32>* %CastToValueType1500.i, align 64
  %38 = getelementptr %struct.WorkDim* %10, i64 0, i32 3, i64 0
  %39 = load i64* %38, align 8
  %40 = trunc i64 %39 to i32
  %41 = icmp eq i32 %40, 0
  %42 = select i1 %41, i32 1, i32 %40
  %43 = sdiv i32 4096, %42
  %temp.i = insertelement <16 x i32> undef, i32 %43, i32 0
  %vector.i = shufflevector <16 x i32> %temp.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %44 = mul nsw i32 %40, %33
  %temp93.i = insertelement <16 x i32> undef, i32 %44, i32 0
  %vector94.i = shufflevector <16 x i32> %temp93.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %45 = mul nsw <16 x i32> %vector.i, %37
  %46 = add nsw <16 x i32> %vector94.i, %45
  %47 = sub nsw <16 x i32> %vector96.i, %46
  %48 = icmp sgt i32 %43, 0
  %temp97.i = insertelement <16 x i1> undef, i1 %48, i32 0
  %vector98.i = shufflevector <16 x i1> %temp97.i, <16 x i1> undef, <16 x i32> zeroinitializer
  %49 = icmp sgt <16 x i32> %47, zeroinitializer
  %or.cond799.i = and <16 x i1> %vector98.i, %49
  %tmp22.i = sub i32 0, %43
  %temp107.i = insertelement <16 x i32> undef, i32 %tmp22.i, i32 0
  %vector108.i = shufflevector <16 x i32> %temp107.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp28105.i = add <16 x i32> %45, %vector94.i
  %tmp29106.i = sub <16 x i32> %tmp28105.i, %vector96.i
  %tmp30.i = icmp ult <16 x i32> %tmp29106.i, %vector108.i
  %umax109.i = select <16 x i1> %tmp30.i, <16 x i32> %vector108.i, <16 x i32> %tmp29106.i
  %tmp31110.i = sub <16 x i32> zeroinitializer, %umax109.i
  %ipred.i.i = bitcast <16 x i1> %or.cond799.i to i16
  %val.i.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i.i, i16 %ipred.i.i) nounwind
  %tmp.i.i = and i32 %val.i.i, 1
  %res.i.i = icmp eq i32 %tmp.i.i, 0
  br i1 %res.i.i, label %.preheader1517.i, label %.critedge.preheader.i

.preheader1517.i:                                 ; preds = %SyncBB1506.i
  %negIncomingLoopMask111.i = xor <16 x i1> %or.cond799.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %51

.critedge.preheader.i:                            ; preds = %postload1114.i, %SyncBB1506.i
  %merge103.i = select <16 x i1> %or.cond799.i, <16 x i32> %tmp31110.i, <16 x i32> zeroinitializer
  %50 = icmp slt <16 x i32> %merge103.i, %vector.i
  %tmp.i = add i32 %43, -1
  %temp173.i = insertelement <16 x i32> undef, i32 %tmp.i, i32 0
  %vector174.i = shufflevector <16 x i32> %temp173.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp11175.i = sub <16 x i32> %vector174.i, %merge103.i
  %tmp12176.i = zext <16 x i32> %tmp11175.i to <16 x i64>
  %tmp13177.i = add <16 x i64> %tmp12176.i, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %tmp17178.i = add <16 x i32> %merge103.i, %45
  %tmp18179.i = zext <16 x i32> %tmp17178.i to <16 x i64>
  %ipred.i1.i = bitcast <16 x i1> %50 to i16
  %val.i2.i = call i32 @llvm.x86.mic.kortestz(i16 %ipred.i1.i, i16 %ipred.i1.i) nounwind
  %tmp.i3.i = and i32 %val.i2.i, 1
  %res.i4.i = icmp eq i32 %tmp.i3.i, 0
  br i1 %res.i4.i, label %.critedge.preheader1518.i, label %bb.nph.i

.critedge.preheader1518.i:                        ; preds = %.critedge.preheader.i
  %negIncomingLoopMask40180.i = xor <16 x i1> %50, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  br label %.critedge.i

; <label>:51                                      ; preds = %postload1114.i, %.preheader1517.i
  %vectorPHI.i = phi <16 x i1> [ %loop_mask8170.i, %postload1114.i ], [ %negIncomingLoopMask111.i, %.preheader1517.i ]
  %vectorPHI113.i = phi <16 x i1> [ %local_edge172.i, %postload1114.i ], [ %or.cond799.i, %.preheader1517.i ]
  %j.08.i = phi i32 [ %148, %postload1114.i ], [ 0, %.preheader1517.i ]
  %extract133.i = extractelement <16 x i1> %vectorPHI113.i, i32 0
  %extract134.i = extractelement <16 x i1> %vectorPHI113.i, i32 1
  %extract135.i = extractelement <16 x i1> %vectorPHI113.i, i32 2
  %extract136.i = extractelement <16 x i1> %vectorPHI113.i, i32 3
  %extract137.i = extractelement <16 x i1> %vectorPHI113.i, i32 4
  %extract138.i = extractelement <16 x i1> %vectorPHI113.i, i32 5
  %extract139.i = extractelement <16 x i1> %vectorPHI113.i, i32 6
  %extract140.i = extractelement <16 x i1> %vectorPHI113.i, i32 7
  %extract141.i = extractelement <16 x i1> %vectorPHI113.i, i32 8
  %extract142.i = extractelement <16 x i1> %vectorPHI113.i, i32 9
  %extract143.i = extractelement <16 x i1> %vectorPHI113.i, i32 10
  %extract144.i = extractelement <16 x i1> %vectorPHI113.i, i32 11
  %extract145.i = extractelement <16 x i1> %vectorPHI113.i, i32 12
  %extract146.i = extractelement <16 x i1> %vectorPHI113.i, i32 13
  %extract147.i = extractelement <16 x i1> %vectorPHI113.i, i32 14
  %extract148.i = extractelement <16 x i1> %vectorPHI113.i, i32 15
  %temp114.i = insertelement <16 x i32> undef, i32 %j.08.i, i32 0
  %vector115.i = shufflevector <16 x i32> %temp114.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %tmp33116.i = add <16 x i32> %tmp28105.i, %vector115.i
  %tmp34117.i = add <16 x i32> %45, %vector115.i
  %52 = extractelement <16 x i32> %tmp33116.i, i32 1
  %53 = sext i32 %52 to i64
  %54 = getelementptr inbounds float addrspace(1)* %1, i64 %53
  %55 = extractelement <16 x i32> %tmp33116.i, i32 2
  %56 = sext i32 %55 to i64
  %57 = getelementptr inbounds float addrspace(1)* %1, i64 %56
  %58 = extractelement <16 x i32> %tmp33116.i, i32 3
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds float addrspace(1)* %1, i64 %59
  %61 = extractelement <16 x i32> %tmp33116.i, i32 4
  %62 = sext i32 %61 to i64
  %63 = getelementptr inbounds float addrspace(1)* %1, i64 %62
  %64 = extractelement <16 x i32> %tmp33116.i, i32 5
  %65 = sext i32 %64 to i64
  %66 = getelementptr inbounds float addrspace(1)* %1, i64 %65
  %67 = extractelement <16 x i32> %tmp33116.i, i32 6
  %68 = sext i32 %67 to i64
  %69 = getelementptr inbounds float addrspace(1)* %1, i64 %68
  %70 = extractelement <16 x i32> %tmp33116.i, i32 7
  %71 = sext i32 %70 to i64
  %72 = getelementptr inbounds float addrspace(1)* %1, i64 %71
  %73 = extractelement <16 x i32> %tmp33116.i, i32 8
  %74 = sext i32 %73 to i64
  %75 = getelementptr inbounds float addrspace(1)* %1, i64 %74
  %76 = extractelement <16 x i32> %tmp33116.i, i32 9
  %77 = sext i32 %76 to i64
  %78 = getelementptr inbounds float addrspace(1)* %1, i64 %77
  %79 = extractelement <16 x i32> %tmp33116.i, i32 10
  %80 = sext i32 %79 to i64
  %81 = getelementptr inbounds float addrspace(1)* %1, i64 %80
  %82 = extractelement <16 x i32> %tmp33116.i, i32 11
  %83 = sext i32 %82 to i64
  %84 = getelementptr inbounds float addrspace(1)* %1, i64 %83
  %85 = extractelement <16 x i32> %tmp33116.i, i32 12
  %86 = sext i32 %85 to i64
  %87 = getelementptr inbounds float addrspace(1)* %1, i64 %86
  %88 = extractelement <16 x i32> %tmp33116.i, i32 13
  %89 = sext i32 %88 to i64
  %90 = getelementptr inbounds float addrspace(1)* %1, i64 %89
  %91 = extractelement <16 x i32> %tmp33116.i, i32 14
  %92 = sext i32 %91 to i64
  %93 = getelementptr inbounds float addrspace(1)* %1, i64 %92
  %94 = extractelement <16 x i32> %tmp33116.i, i32 15
  %95 = sext i32 %94 to i64
  %96 = getelementptr inbounds float addrspace(1)* %1, i64 %95
  br i1 %extract133.i, label %preload1403.i, label %postload1404.i

preload1403.i:                                    ; preds = %51
  %97 = extractelement <16 x i32> %tmp33116.i, i32 0
  %98 = sext i32 %97 to i64
  %99 = getelementptr inbounds float addrspace(1)* %1, i64 %98
  %masked_load.i = load float addrspace(1)* %99, align 4
  br label %postload1404.i

postload1404.i:                                   ; preds = %preload1403.i, %51
  %phi1405.i = phi float [ undef, %51 ], [ %masked_load.i, %preload1403.i ]
  br i1 %extract134.i, label %preload1408.i, label %postload1409.i

preload1408.i:                                    ; preds = %postload1404.i
  %masked_load761.i = load float addrspace(1)* %54, align 4
  br label %postload1409.i

postload1409.i:                                   ; preds = %preload1408.i, %postload1404.i
  %phi1410.i = phi float [ undef, %postload1404.i ], [ %masked_load761.i, %preload1408.i ]
  br i1 %extract135.i, label %preload.i, label %postload.i

preload.i:                                        ; preds = %postload1409.i
  %masked_load762.i = load float addrspace(1)* %57, align 4
  br label %postload.i

postload.i:                                       ; preds = %preload.i, %postload1409.i
  %phi.i = phi float [ undef, %postload1409.i ], [ %masked_load762.i, %preload.i ]
  br i1 %extract136.i, label %preload1050.i, label %postload1051.i

preload1050.i:                                    ; preds = %postload.i
  %masked_load763.i = load float addrspace(1)* %60, align 4
  br label %postload1051.i

postload1051.i:                                   ; preds = %preload1050.i, %postload.i
  %phi1052.i = phi float [ undef, %postload.i ], [ %masked_load763.i, %preload1050.i ]
  br i1 %extract137.i, label %preload1055.i, label %postload1056.i

preload1055.i:                                    ; preds = %postload1051.i
  %masked_load764.i = load float addrspace(1)* %63, align 4
  br label %postload1056.i

postload1056.i:                                   ; preds = %preload1055.i, %postload1051.i
  %phi1057.i = phi float [ undef, %postload1051.i ], [ %masked_load764.i, %preload1055.i ]
  br i1 %extract138.i, label %preload1060.i, label %postload1061.i

preload1060.i:                                    ; preds = %postload1056.i
  %masked_load765.i = load float addrspace(1)* %66, align 4
  br label %postload1061.i

postload1061.i:                                   ; preds = %preload1060.i, %postload1056.i
  %phi1062.i = phi float [ undef, %postload1056.i ], [ %masked_load765.i, %preload1060.i ]
  br i1 %extract139.i, label %preload1065.i, label %postload1066.i

preload1065.i:                                    ; preds = %postload1061.i
  %masked_load766.i = load float addrspace(1)* %69, align 4
  br label %postload1066.i

postload1066.i:                                   ; preds = %preload1065.i, %postload1061.i
  %phi1067.i = phi float [ undef, %postload1061.i ], [ %masked_load766.i, %preload1065.i ]
  br i1 %extract140.i, label %preload1070.i, label %postload1071.i

preload1070.i:                                    ; preds = %postload1066.i
  %masked_load767.i = load float addrspace(1)* %72, align 4
  br label %postload1071.i

postload1071.i:                                   ; preds = %preload1070.i, %postload1066.i
  %phi1072.i = phi float [ undef, %postload1066.i ], [ %masked_load767.i, %preload1070.i ]
  br i1 %extract141.i, label %preload1075.i, label %postload1076.i

preload1075.i:                                    ; preds = %postload1071.i
  %masked_load768.i = load float addrspace(1)* %75, align 4
  br label %postload1076.i

postload1076.i:                                   ; preds = %preload1075.i, %postload1071.i
  %phi1077.i = phi float [ undef, %postload1071.i ], [ %masked_load768.i, %preload1075.i ]
  br i1 %extract142.i, label %preload1080.i, label %postload1081.i

preload1080.i:                                    ; preds = %postload1076.i
  %masked_load769.i = load float addrspace(1)* %78, align 4
  br label %postload1081.i

postload1081.i:                                   ; preds = %preload1080.i, %postload1076.i
  %phi1082.i = phi float [ undef, %postload1076.i ], [ %masked_load769.i, %preload1080.i ]
  br i1 %extract143.i, label %preload1085.i, label %postload1086.i

preload1085.i:                                    ; preds = %postload1081.i
  %masked_load770.i = load float addrspace(1)* %81, align 4
  br label %postload1086.i

postload1086.i:                                   ; preds = %preload1085.i, %postload1081.i
  %phi1087.i = phi float [ undef, %postload1081.i ], [ %masked_load770.i, %preload1085.i ]
  br i1 %extract144.i, label %preload1090.i, label %postload1091.i

preload1090.i:                                    ; preds = %postload1086.i
  %masked_load771.i = load float addrspace(1)* %84, align 4
  br label %postload1091.i

postload1091.i:                                   ; preds = %preload1090.i, %postload1086.i
  %phi1092.i = phi float [ undef, %postload1086.i ], [ %masked_load771.i, %preload1090.i ]
  br i1 %extract145.i, label %preload1095.i, label %postload1096.i

preload1095.i:                                    ; preds = %postload1091.i
  %masked_load772.i = load float addrspace(1)* %87, align 4
  br label %postload1096.i

postload1096.i:                                   ; preds = %preload1095.i, %postload1091.i
  %phi1097.i = phi float [ undef, %postload1091.i ], [ %masked_load772.i, %preload1095.i ]
  br i1 %extract146.i, label %preload1100.i, label %postload1101.i

preload1100.i:                                    ; preds = %postload1096.i
  %masked_load773.i = load float addrspace(1)* %90, align 4
  br label %postload1101.i

postload1101.i:                                   ; preds = %preload1100.i, %postload1096.i
  %phi1102.i = phi float [ undef, %postload1096.i ], [ %masked_load773.i, %preload1100.i ]
  br i1 %extract147.i, label %preload1105.i, label %postload1106.i

preload1105.i:                                    ; preds = %postload1101.i
  %masked_load774.i = load float addrspace(1)* %93, align 4
  br label %postload1106.i

postload1106.i:                                   ; preds = %preload1105.i, %postload1101.i
  %phi1107.i = phi float [ undef, %postload1101.i ], [ %masked_load774.i, %preload1105.i ]
  br i1 %extract148.i, label %preload1110.i, label %postload1111.i

preload1110.i:                                    ; preds = %postload1106.i
  %masked_load775.i = load float addrspace(1)* %96, align 4
  br label %postload1111.i

postload1111.i:                                   ; preds = %preload1110.i, %postload1106.i
  %phi1112.i = phi float [ undef, %postload1106.i ], [ %masked_load775.i, %preload1110.i ]
  %100 = extractelement <16 x i32> %tmp34117.i, i32 1
  %101 = sext i32 %100 to i64
  %102 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %101
  %103 = extractelement <16 x i32> %tmp34117.i, i32 2
  %104 = sext i32 %103 to i64
  %105 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %104
  %106 = extractelement <16 x i32> %tmp34117.i, i32 3
  %107 = sext i32 %106 to i64
  %108 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %107
  %109 = extractelement <16 x i32> %tmp34117.i, i32 4
  %110 = sext i32 %109 to i64
  %111 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %110
  %112 = extractelement <16 x i32> %tmp34117.i, i32 5
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %113
  %115 = extractelement <16 x i32> %tmp34117.i, i32 6
  %116 = sext i32 %115 to i64
  %117 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %116
  %118 = extractelement <16 x i32> %tmp34117.i, i32 7
  %119 = sext i32 %118 to i64
  %120 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %119
  %121 = extractelement <16 x i32> %tmp34117.i, i32 8
  %122 = sext i32 %121 to i64
  %123 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %122
  %124 = extractelement <16 x i32> %tmp34117.i, i32 9
  %125 = sext i32 %124 to i64
  %126 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %125
  %127 = extractelement <16 x i32> %tmp34117.i, i32 10
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %128
  %130 = extractelement <16 x i32> %tmp34117.i, i32 11
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %131
  %133 = extractelement <16 x i32> %tmp34117.i, i32 12
  %134 = sext i32 %133 to i64
  %135 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %134
  %136 = extractelement <16 x i32> %tmp34117.i, i32 13
  %137 = sext i32 %136 to i64
  %138 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %137
  %139 = extractelement <16 x i32> %tmp34117.i, i32 14
  %140 = sext i32 %139 to i64
  %141 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %140
  %142 = extractelement <16 x i32> %tmp34117.i, i32 15
  %143 = sext i32 %142 to i64
  %144 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %143
  br i1 %extract133.i, label %preload1406.i, label %postload1407.i

preload1406.i:                                    ; preds = %postload1111.i
  %145 = extractelement <16 x i32> %tmp34117.i, i32 0
  %146 = sext i32 %145 to i64
  %147 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %146
  store float %phi1405.i, float addrspace(3)* %147, align 4
  br label %postload1407.i

postload1407.i:                                   ; preds = %preload1406.i, %postload1111.i
  br i1 %extract134.i, label %preload1411.i, label %postload1412.i

preload1411.i:                                    ; preds = %postload1407.i
  store float %phi1410.i, float addrspace(3)* %102, align 4
  br label %postload1412.i

postload1412.i:                                   ; preds = %preload1411.i, %postload1407.i
  br i1 %extract135.i, label %preload1048.i, label %postload1049.i

preload1048.i:                                    ; preds = %postload1412.i
  store float %phi.i, float addrspace(3)* %105, align 4
  br label %postload1049.i

postload1049.i:                                   ; preds = %preload1048.i, %postload1412.i
  br i1 %extract136.i, label %preload1053.i, label %postload1054.i

preload1053.i:                                    ; preds = %postload1049.i
  store float %phi1052.i, float addrspace(3)* %108, align 4
  br label %postload1054.i

postload1054.i:                                   ; preds = %preload1053.i, %postload1049.i
  br i1 %extract137.i, label %preload1058.i, label %postload1059.i

preload1058.i:                                    ; preds = %postload1054.i
  store float %phi1057.i, float addrspace(3)* %111, align 4
  br label %postload1059.i

postload1059.i:                                   ; preds = %preload1058.i, %postload1054.i
  br i1 %extract138.i, label %preload1063.i, label %postload1064.i

preload1063.i:                                    ; preds = %postload1059.i
  store float %phi1062.i, float addrspace(3)* %114, align 4
  br label %postload1064.i

postload1064.i:                                   ; preds = %preload1063.i, %postload1059.i
  br i1 %extract139.i, label %preload1068.i, label %postload1069.i

preload1068.i:                                    ; preds = %postload1064.i
  store float %phi1067.i, float addrspace(3)* %117, align 4
  br label %postload1069.i

postload1069.i:                                   ; preds = %preload1068.i, %postload1064.i
  br i1 %extract140.i, label %preload1073.i, label %postload1074.i

preload1073.i:                                    ; preds = %postload1069.i
  store float %phi1072.i, float addrspace(3)* %120, align 4
  br label %postload1074.i

postload1074.i:                                   ; preds = %preload1073.i, %postload1069.i
  br i1 %extract141.i, label %preload1078.i, label %postload1079.i

preload1078.i:                                    ; preds = %postload1074.i
  store float %phi1077.i, float addrspace(3)* %123, align 4
  br label %postload1079.i

postload1079.i:                                   ; preds = %preload1078.i, %postload1074.i
  br i1 %extract142.i, label %preload1083.i, label %postload1084.i

preload1083.i:                                    ; preds = %postload1079.i
  store float %phi1082.i, float addrspace(3)* %126, align 4
  br label %postload1084.i

postload1084.i:                                   ; preds = %preload1083.i, %postload1079.i
  br i1 %extract143.i, label %preload1088.i, label %postload1089.i

preload1088.i:                                    ; preds = %postload1084.i
  store float %phi1087.i, float addrspace(3)* %129, align 4
  br label %postload1089.i

postload1089.i:                                   ; preds = %preload1088.i, %postload1084.i
  br i1 %extract144.i, label %preload1093.i, label %postload1094.i

preload1093.i:                                    ; preds = %postload1089.i
  store float %phi1092.i, float addrspace(3)* %132, align 4
  br label %postload1094.i

postload1094.i:                                   ; preds = %preload1093.i, %postload1089.i
  br i1 %extract145.i, label %preload1098.i, label %postload1099.i

preload1098.i:                                    ; preds = %postload1094.i
  store float %phi1097.i, float addrspace(3)* %135, align 4
  br label %postload1099.i

postload1099.i:                                   ; preds = %preload1098.i, %postload1094.i
  br i1 %extract146.i, label %preload1103.i, label %postload1104.i

preload1103.i:                                    ; preds = %postload1099.i
  store float %phi1102.i, float addrspace(3)* %138, align 4
  br label %postload1104.i

postload1104.i:                                   ; preds = %preload1103.i, %postload1099.i
  br i1 %extract147.i, label %preload1108.i, label %postload1109.i

preload1108.i:                                    ; preds = %postload1104.i
  store float %phi1107.i, float addrspace(3)* %141, align 4
  br label %postload1109.i

postload1109.i:                                   ; preds = %preload1108.i, %postload1104.i
  br i1 %extract148.i, label %preload1113.i, label %postload1114.i

preload1113.i:                                    ; preds = %postload1109.i
  store float %phi1112.i, float addrspace(3)* %144, align 4
  br label %postload1114.i

postload1114.i:                                   ; preds = %preload1113.i, %postload1109.i
  %148 = add nsw i32 %j.08.i, 1
  %temp165.i = insertelement <16 x i32> undef, i32 %148, i32 0
  %vector166.i = shufflevector <16 x i32> %temp165.i, <16 x i32> undef, <16 x i32> zeroinitializer
  %exitcond32.i = icmp eq <16 x i32> %vector166.i, %tmp31110.i
  %notCond167.i = xor <16 x i1> %exitcond32.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr168.i = and <16 x i1> %vectorPHI113.i, %exitcond32.i
  %loop_mask8170.i = or <16 x i1> %vectorPHI.i, %who_left_tr168.i
  %curr_loop_mask171.i = or <16 x i1> %loop_mask8170.i, %who_left_tr168.i
  %ipred.i5.i = bitcast <16 x i1> %curr_loop_mask171.i to i16
  %val.i6.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i5.i, i16 %ipred.i5.i) nounwind
  %tmp.i7.i = and i32 %val.i6.i, 1
  %res.i8.i = icmp eq i32 %tmp.i7.i, 0
  %local_edge172.i = and <16 x i1> %vectorPHI113.i, %notCond167.i
  br i1 %res.i8.i, label %51, label %.critedge.preheader.i

.critedge.i:                                      ; preds = %postload1492.i, %.critedge.preheader1518.i
  %vectorPHI181.i = phi <16 x i1> [ %loop_mask15225.i, %postload1492.i ], [ %negIncomingLoopMask40180.i, %.critedge.preheader1518.i ]
  %vectorPHI183.i = phi <16 x i1> [ %local_edge20227.i, %postload1492.i ], [ %50, %.critedge.preheader1518.i ]
  %indvar.i = phi i64 [ %indvar.next.i, %postload1492.i ], [ 0, %.critedge.preheader1518.i ]
  %extract187.i = extractelement <16 x i1> %vectorPHI183.i, i32 0
  %extract188.i = extractelement <16 x i1> %vectorPHI183.i, i32 1
  %extract189.i = extractelement <16 x i1> %vectorPHI183.i, i32 2
  %extract190.i = extractelement <16 x i1> %vectorPHI183.i, i32 3
  %extract191.i = extractelement <16 x i1> %vectorPHI183.i, i32 4
  %extract192.i = extractelement <16 x i1> %vectorPHI183.i, i32 5
  %extract193.i = extractelement <16 x i1> %vectorPHI183.i, i32 6
  %extract194.i = extractelement <16 x i1> %vectorPHI183.i, i32 7
  %extract195.i = extractelement <16 x i1> %vectorPHI183.i, i32 8
  %extract196.i = extractelement <16 x i1> %vectorPHI183.i, i32 9
  %extract197.i = extractelement <16 x i1> %vectorPHI183.i, i32 10
  %extract198.i = extractelement <16 x i1> %vectorPHI183.i, i32 11
  %extract199.i = extractelement <16 x i1> %vectorPHI183.i, i32 12
  %extract200.i = extractelement <16 x i1> %vectorPHI183.i, i32 13
  %extract201.i = extractelement <16 x i1> %vectorPHI183.i, i32 14
  %extract202.i = extractelement <16 x i1> %vectorPHI183.i, i32 15
  %temp184.i = insertelement <16 x i64> undef, i64 %indvar.i, i32 0
  %vector185.i = shufflevector <16 x i64> %temp184.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %scevgep.i = getelementptr float addrspace(1)* %1, i64 %indvar.i
  %tmp19186.i = add <16 x i64> %tmp18179.i, %vector185.i
  br i1 %extract187.i, label %preload1413.i, label %postload1414.i

preload1413.i:                                    ; preds = %.critedge.i
  %masked_load776.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1414.i

postload1414.i:                                   ; preds = %preload1413.i, %.critedge.i
  %phi1415.i = phi float [ undef, %.critedge.i ], [ %masked_load776.i, %preload1413.i ]
  br i1 %extract188.i, label %preload1418.i, label %postload1419.i

preload1418.i:                                    ; preds = %postload1414.i
  %masked_load777.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1419.i

postload1419.i:                                   ; preds = %preload1418.i, %postload1414.i
  %phi1420.i = phi float [ undef, %postload1414.i ], [ %masked_load777.i, %preload1418.i ]
  br i1 %extract189.i, label %preload1423.i, label %postload1424.i

preload1423.i:                                    ; preds = %postload1419.i
  %masked_load778.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1424.i

postload1424.i:                                   ; preds = %preload1423.i, %postload1419.i
  %phi1425.i = phi float [ undef, %postload1419.i ], [ %masked_load778.i, %preload1423.i ]
  br i1 %extract190.i, label %preload1428.i, label %postload1429.i

preload1428.i:                                    ; preds = %postload1424.i
  %masked_load779.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1429.i

postload1429.i:                                   ; preds = %preload1428.i, %postload1424.i
  %phi1430.i = phi float [ undef, %postload1424.i ], [ %masked_load779.i, %preload1428.i ]
  br i1 %extract191.i, label %preload1433.i, label %postload1434.i

preload1433.i:                                    ; preds = %postload1429.i
  %masked_load780.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1434.i

postload1434.i:                                   ; preds = %preload1433.i, %postload1429.i
  %phi1435.i = phi float [ undef, %postload1429.i ], [ %masked_load780.i, %preload1433.i ]
  br i1 %extract192.i, label %preload1438.i, label %postload1439.i

preload1438.i:                                    ; preds = %postload1434.i
  %masked_load781.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1439.i

postload1439.i:                                   ; preds = %preload1438.i, %postload1434.i
  %phi1440.i = phi float [ undef, %postload1434.i ], [ %masked_load781.i, %preload1438.i ]
  br i1 %extract193.i, label %preload1443.i, label %postload1444.i

preload1443.i:                                    ; preds = %postload1439.i
  %masked_load782.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1444.i

postload1444.i:                                   ; preds = %preload1443.i, %postload1439.i
  %phi1445.i = phi float [ undef, %postload1439.i ], [ %masked_load782.i, %preload1443.i ]
  br i1 %extract194.i, label %preload1448.i, label %postload1449.i

preload1448.i:                                    ; preds = %postload1444.i
  %masked_load783.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1449.i

postload1449.i:                                   ; preds = %preload1448.i, %postload1444.i
  %phi1450.i = phi float [ undef, %postload1444.i ], [ %masked_load783.i, %preload1448.i ]
  br i1 %extract195.i, label %preload1453.i, label %postload1454.i

preload1453.i:                                    ; preds = %postload1449.i
  %masked_load784.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1454.i

postload1454.i:                                   ; preds = %preload1453.i, %postload1449.i
  %phi1455.i = phi float [ undef, %postload1449.i ], [ %masked_load784.i, %preload1453.i ]
  br i1 %extract196.i, label %preload1458.i, label %postload1459.i

preload1458.i:                                    ; preds = %postload1454.i
  %masked_load785.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1459.i

postload1459.i:                                   ; preds = %preload1458.i, %postload1454.i
  %phi1460.i = phi float [ undef, %postload1454.i ], [ %masked_load785.i, %preload1458.i ]
  br i1 %extract197.i, label %preload1463.i, label %postload1464.i

preload1463.i:                                    ; preds = %postload1459.i
  %masked_load786.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1464.i

postload1464.i:                                   ; preds = %preload1463.i, %postload1459.i
  %phi1465.i = phi float [ undef, %postload1459.i ], [ %masked_load786.i, %preload1463.i ]
  br i1 %extract198.i, label %preload1468.i, label %postload1469.i

preload1468.i:                                    ; preds = %postload1464.i
  %masked_load787.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1469.i

postload1469.i:                                   ; preds = %preload1468.i, %postload1464.i
  %phi1470.i = phi float [ undef, %postload1464.i ], [ %masked_load787.i, %preload1468.i ]
  br i1 %extract199.i, label %preload1473.i, label %postload1474.i

preload1473.i:                                    ; preds = %postload1469.i
  %masked_load788.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1474.i

postload1474.i:                                   ; preds = %preload1473.i, %postload1469.i
  %phi1475.i = phi float [ undef, %postload1469.i ], [ %masked_load788.i, %preload1473.i ]
  br i1 %extract200.i, label %preload1478.i, label %postload1479.i

preload1478.i:                                    ; preds = %postload1474.i
  %masked_load789.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1479.i

postload1479.i:                                   ; preds = %preload1478.i, %postload1474.i
  %phi1480.i = phi float [ undef, %postload1474.i ], [ %masked_load789.i, %preload1478.i ]
  br i1 %extract201.i, label %preload1483.i, label %postload1484.i

preload1483.i:                                    ; preds = %postload1479.i
  %masked_load790.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1484.i

postload1484.i:                                   ; preds = %preload1483.i, %postload1479.i
  %phi1485.i = phi float [ undef, %postload1479.i ], [ %masked_load790.i, %preload1483.i ]
  br i1 %extract202.i, label %preload1488.i, label %postload1489.i

preload1488.i:                                    ; preds = %postload1484.i
  %masked_load791.i = load float addrspace(1)* %scevgep.i, align 4
  br label %postload1489.i

postload1489.i:                                   ; preds = %preload1488.i, %postload1484.i
  %phi1490.i = phi float [ undef, %postload1484.i ], [ %masked_load791.i, %preload1488.i ]
  %sext35203.i = shl <16 x i64> %tmp19186.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %149 = ashr <16 x i64> %sext35203.i, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %extract205.i = extractelement <16 x i64> %149, i32 1
  %extract206.i = extractelement <16 x i64> %149, i32 2
  %extract207.i = extractelement <16 x i64> %149, i32 3
  %extract208.i = extractelement <16 x i64> %149, i32 4
  %extract209.i = extractelement <16 x i64> %149, i32 5
  %extract210.i = extractelement <16 x i64> %149, i32 6
  %extract211.i = extractelement <16 x i64> %149, i32 7
  %extract212.i = extractelement <16 x i64> %149, i32 8
  %extract213.i = extractelement <16 x i64> %149, i32 9
  %extract214.i = extractelement <16 x i64> %149, i32 10
  %extract215.i = extractelement <16 x i64> %149, i32 11
  %extract216.i = extractelement <16 x i64> %149, i32 12
  %extract217.i = extractelement <16 x i64> %149, i32 13
  %extract218.i = extractelement <16 x i64> %149, i32 14
  %extract219.i = extractelement <16 x i64> %149, i32 15
  %150 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract205.i
  %151 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract206.i
  %152 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract207.i
  %153 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract208.i
  %154 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract209.i
  %155 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract210.i
  %156 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract211.i
  %157 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract212.i
  %158 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract213.i
  %159 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract214.i
  %160 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract215.i
  %161 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract216.i
  %162 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract217.i
  %163 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract218.i
  %164 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract219.i
  br i1 %extract187.i, label %preload1416.i, label %postload1417.i

preload1416.i:                                    ; preds = %postload1489.i
  %extract204.i = extractelement <16 x i64> %149, i32 0
  %165 = getelementptr inbounds [4096 x float] addrspace(3)* %26, i64 0, i64 %extract204.i
  store float %phi1415.i, float addrspace(3)* %165, align 4
  br label %postload1417.i

postload1417.i:                                   ; preds = %preload1416.i, %postload1489.i
  br i1 %extract188.i, label %preload1421.i, label %postload1422.i

preload1421.i:                                    ; preds = %postload1417.i
  store float %phi1420.i, float addrspace(3)* %150, align 4
  br label %postload1422.i

postload1422.i:                                   ; preds = %preload1421.i, %postload1417.i
  br i1 %extract189.i, label %preload1426.i, label %postload1427.i

preload1426.i:                                    ; preds = %postload1422.i
  store float %phi1425.i, float addrspace(3)* %151, align 4
  br label %postload1427.i

postload1427.i:                                   ; preds = %preload1426.i, %postload1422.i
  br i1 %extract190.i, label %preload1431.i, label %postload1432.i

preload1431.i:                                    ; preds = %postload1427.i
  store float %phi1430.i, float addrspace(3)* %152, align 4
  br label %postload1432.i

postload1432.i:                                   ; preds = %preload1431.i, %postload1427.i
  br i1 %extract191.i, label %preload1436.i, label %postload1437.i

preload1436.i:                                    ; preds = %postload1432.i
  store float %phi1435.i, float addrspace(3)* %153, align 4
  br label %postload1437.i

postload1437.i:                                   ; preds = %preload1436.i, %postload1432.i
  br i1 %extract192.i, label %preload1441.i, label %postload1442.i

preload1441.i:                                    ; preds = %postload1437.i
  store float %phi1440.i, float addrspace(3)* %154, align 4
  br label %postload1442.i

postload1442.i:                                   ; preds = %preload1441.i, %postload1437.i
  br i1 %extract193.i, label %preload1446.i, label %postload1447.i

preload1446.i:                                    ; preds = %postload1442.i
  store float %phi1445.i, float addrspace(3)* %155, align 4
  br label %postload1447.i

postload1447.i:                                   ; preds = %preload1446.i, %postload1442.i
  br i1 %extract194.i, label %preload1451.i, label %postload1452.i

preload1451.i:                                    ; preds = %postload1447.i
  store float %phi1450.i, float addrspace(3)* %156, align 4
  br label %postload1452.i

postload1452.i:                                   ; preds = %preload1451.i, %postload1447.i
  br i1 %extract195.i, label %preload1456.i, label %postload1457.i

preload1456.i:                                    ; preds = %postload1452.i
  store float %phi1455.i, float addrspace(3)* %157, align 4
  br label %postload1457.i

postload1457.i:                                   ; preds = %preload1456.i, %postload1452.i
  br i1 %extract196.i, label %preload1461.i, label %postload1462.i

preload1461.i:                                    ; preds = %postload1457.i
  store float %phi1460.i, float addrspace(3)* %158, align 4
  br label %postload1462.i

postload1462.i:                                   ; preds = %preload1461.i, %postload1457.i
  br i1 %extract197.i, label %preload1466.i, label %postload1467.i

preload1466.i:                                    ; preds = %postload1462.i
  store float %phi1465.i, float addrspace(3)* %159, align 4
  br label %postload1467.i

postload1467.i:                                   ; preds = %preload1466.i, %postload1462.i
  br i1 %extract198.i, label %preload1471.i, label %postload1472.i

preload1471.i:                                    ; preds = %postload1467.i
  store float %phi1470.i, float addrspace(3)* %160, align 4
  br label %postload1472.i

postload1472.i:                                   ; preds = %preload1471.i, %postload1467.i
  br i1 %extract199.i, label %preload1476.i, label %postload1477.i

preload1476.i:                                    ; preds = %postload1472.i
  store float %phi1475.i, float addrspace(3)* %161, align 4
  br label %postload1477.i

postload1477.i:                                   ; preds = %preload1476.i, %postload1472.i
  br i1 %extract200.i, label %preload1481.i, label %postload1482.i

preload1481.i:                                    ; preds = %postload1477.i
  store float %phi1480.i, float addrspace(3)* %162, align 4
  br label %postload1482.i

postload1482.i:                                   ; preds = %preload1481.i, %postload1477.i
  br i1 %extract201.i, label %preload1486.i, label %postload1487.i

preload1486.i:                                    ; preds = %postload1482.i
  store float %phi1485.i, float addrspace(3)* %163, align 4
  br label %postload1487.i

postload1487.i:                                   ; preds = %preload1486.i, %postload1482.i
  br i1 %extract202.i, label %preload1491.i, label %postload1492.i

preload1491.i:                                    ; preds = %postload1487.i
  store float %phi1490.i, float addrspace(3)* %164, align 4
  br label %postload1492.i

postload1492.i:                                   ; preds = %preload1491.i, %postload1487.i
  %indvar.next.i = add i64 %indvar.i, 1
  %temp220.i = insertelement <16 x i64> undef, i64 %indvar.next.i, i32 0
  %vector221.i = shufflevector <16 x i64> %temp220.i, <16 x i64> undef, <16 x i32> zeroinitializer
  %exitcond14.i = icmp eq <16 x i64> %vector221.i, %tmp13177.i
  %notCond11222.i = xor <16 x i1> %exitcond14.i, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %who_left_tr12223.i = and <16 x i1> %vectorPHI183.i, %exitcond14.i
  %loop_mask15225.i = or <16 x i1> %vectorPHI181.i, %who_left_tr12223.i
  %curr_loop_mask17226.i = or <16 x i1> %loop_mask15225.i, %who_left_tr12223.i
  %ipred.i9.i = bitcast <16 x i1> %curr_loop_mask17226.i to i16
  %val.i10.i = call i32 @llvm.x86.mic.kortestc(i16 %ipred.i9.i, i16 %ipred.i9.i) nounwind
  %tmp.i11.i = and i32 %val.i10.i, 1
  %res.i12.i = icmp eq i32 %tmp.i11.i, 0
  %local_edge20227.i = and <16 x i1> %vectorPHI183.i, %notCond11222.i
  br i1 %res.i12.i, label %.critedge.i, label %bb.nph.i

bb.nph.i:                                         ; preds = %postload1492.i, %.critedge.preheader.i
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %22
  br i1 %check.WI.iter.i, label %thenBB.i, label %"Barrier BB.i"

thenBB.i:                                         ; preds = %bb.nph.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 128
  br label %SyncBB1506.i

"Barrier BB.i":                                   ; preds = %thenBB1509.i, %bb.nph.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++1513.i", %thenBB1509.i ], [ 0, %bb.nph.i ]
  %check.WI.iter1512.i = icmp ult i64 %CurrWI..1.i, %22
  br i1 %check.WI.iter1512.i, label %thenBB1509.i, label %____Vectorized_.readLocalMemory_separated_args.exit

thenBB1509.i:                                     ; preds = %"Barrier BB.i"
  %"CurrWI++1513.i" = add nuw i64 %CurrWI..1.i, 1
  br label %"Barrier BB.i"

____Vectorized_.readLocalMemory_separated_args.exit: ; preds = %"Barrier BB.i"
  ret void
}

!opencl.kernels = !{!0}
!opencl_readLocalMemory_locals_anchor = !{!2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readLocalMemory_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int", metadata !"opencl_readLocalMemory_locals_anchor", void (i8*)* @readLocalMemory}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"opencl_readLocalMemory_local_lbuf"}
