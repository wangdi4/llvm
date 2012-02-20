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

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__readLocalMemory_separated_args(float addrspace(1)* nocapture %data, float addrspace(1)* nocapture %output, i32 %size, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
; <label>:0
  %1 = bitcast i8 addrspace(3)* %pLocalMem to [4096 x float] addrspace(3)*
  br label %SyncBB49

SyncBB49:                                         ; preds = %0, %thenBB52
  %CurrWI..1 = phi i64 [ %"CurrWI++56", %thenBB52 ], [ 0, %0 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride58", %thenBB52 ], [ 0, %0 ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  store i64 %6, i64* %CastToValueType, align 8
  %7 = load i64* %pWGId, align 8
  %8 = trunc i64 %7 to i32
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..1, i32 0, i64 0
  %10 = load i64* %9, align 8
  %11 = trunc i64 %10 to i32
  %"&(pSB[currWI].offset)411" = or i64 %CurrSBIndex..1, 8
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
  %check.WI.iter55 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter55, label %thenBB52, label %SyncBB50

thenBB52:                                         ; preds = %bb.nph
  %"CurrWI++56" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride58" = add nuw i64 %CurrSBIndex..1, 16
  br label %SyncBB49

SyncBB50:                                         ; preds = %bb.nph, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %bb.nph ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %bb.nph ]
  %"&(pSB[currWI].offset)452" = or i64 %CurrSBIndex..0, 8
  %"&pSB[currWI].offset46" = getelementptr inbounds i8* %pSpecialBuf, i64 %"&(pSB[currWI].offset)452"
  %CastToValueType47 = bitcast i8* %"&pSB[currWI].offset46" to i32*
  %loadedValue48 = load i32* %CastToValueType47, align 4
  br label %35

; <label>:35                                      ; preds = %35, %SyncBB50
  %s.03 = phi i32 [ %loadedValue48, %SyncBB50 ], [ %133, %35 ]
  %sum.02 = phi float [ 0.000000e+00, %SyncBB50 ], [ %130, %35 ]
  %j.21 = phi i32 [ 0, %SyncBB50 ], [ %131, %35 ]
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
  %"&pSB[currWI].offset38" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType39 = bitcast i8* %"&pSB[currWI].offset38" to i64*
  %loadedValue = load i64* %CastToValueType39, align 8
  %sext = shl i64 %loadedValue, 32
  %134 = ashr i64 %sext, 32
  %135 = getelementptr inbounds float addrspace(1)* %output, i64 %134
  store float %130, float addrspace(1)* %135, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %._crit_edge
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 16
  br label %SyncBB50

SyncBB:                                           ; preds = %._crit_edge
  ret void
}

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

SyncBB49.i:                                       ; preds = %thenBB52.i, %entry
  %CurrWI..1.i = phi i64 [ %"CurrWI++56.i", %thenBB52.i ], [ 0, %entry ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride58.i", %thenBB52.i ], [ 0, %entry ]
  %30 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..1.i, i32 0, i64 0
  %31 = load i64* %30, align 8
  %32 = getelementptr %struct.PaddedDimId* %19, i64 0, i32 0, i64 0
  %33 = load i64* %32, align 8
  %34 = add i64 %31, %33
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..1.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  store i64 %34, i64* %CastToValueType.i, align 8
  %35 = load i64* %16, align 8
  %36 = trunc i64 %35 to i32
  %37 = getelementptr %struct.PaddedDimId* %22, i64 %CurrWI..1.i, i32 0, i64 0
  %38 = load i64* %37, align 8
  %39 = trunc i64 %38 to i32
  %"&(pSB[currWI].offset)411.i" = or i64 %CurrSBIndex..1.i, 8
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
  %check.WI.iter55.i = icmp ult i64 %CurrWI..1.i, %25
  br i1 %check.WI.iter55.i, label %thenBB52.i, label %SyncBB50.i

thenBB52.i:                                       ; preds = %bb.nph.i
  %"CurrWI++56.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride58.i" = add nuw i64 %CurrSBIndex..1.i, 16
  br label %SyncBB49.i

SyncBB50.i:                                       ; preds = %thenBB.i, %bb.nph.i
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %bb.nph.i ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %bb.nph.i ]
  %"&(pSB[currWI].offset)452.i" = or i64 %CurrSBIndex..0.i, 8
  %"&pSB[currWI].offset46.i" = getelementptr inbounds i8* %28, i64 %"&(pSB[currWI].offset)452.i"
  %CastToValueType47.i = bitcast i8* %"&pSB[currWI].offset46.i" to i32*
  %loadedValue48.i = load i32* %CastToValueType47.i, align 4
  br label %63

; <label>:63                                      ; preds = %63, %SyncBB50.i
  %s.03.i = phi i32 [ %loadedValue48.i, %SyncBB50.i ], [ %161, %63 ]
  %sum.02.i = phi float [ 0.000000e+00, %SyncBB50.i ], [ %158, %63 ]
  %j.21.i = phi i32 [ 0, %SyncBB50.i ], [ %159, %63 ]
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
  %"&pSB[currWI].offset38.i" = getelementptr inbounds i8* %28, i64 %CurrSBIndex..0.i
  %CastToValueType39.i = bitcast i8* %"&pSB[currWI].offset38.i" to i64*
  %loadedValue.i = load i64* %CastToValueType39.i, align 8
  %sext.i = shl i64 %loadedValue.i, 32
  %162 = ashr i64 %sext.i, 32
  %163 = getelementptr inbounds float addrspace(1)* %4, i64 %162
  store float %158, float addrspace(1)* %163, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %25
  br i1 %check.WI.iter.i, label %thenBB.i, label %__readLocalMemory_separated_args.exit

thenBB.i:                                         ; preds = %._crit_edge.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 16
  br label %SyncBB50.i

__readLocalMemory_separated_args.exit:            ; preds = %._crit_edge.i
  ret void
}

!opencl.kernels = !{!0}
!opencl_readLocalMemory_locals_anchor = !{!2}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__readLocalMemory_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int", metadata !"opencl_readLocalMemory_locals_anchor", void (i8*)* @readLocalMemory}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"opencl_readLocalMemory_local_lbuf"}
