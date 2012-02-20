; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__get_global_size_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_global_size(i32) readnone

declare i64 @get_global_id(i32) readnone

declare void @__get_global_id_test_original(i64 addrspace(1)* nocapture) nounwind

declare void @__get_local_size_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_local_size(i32) readnone

declare void @__get_local_id_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_local_id(i32) readnone

declare void @__get_num_groups_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_num_groups(i32) readnone

declare void @__get_group_id_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_group_id(i32) readnone

declare void @__get_global_offset_test_original(i64 addrspace(1)* nocapture) nounwind

declare i64 @get_global_offset(i32) readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_local_id.(i32, i64)

declare i64 @get_new_global_id.(i32, i64)

define void @__get_global_size_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  br label %SyncBB1

SyncBB1:                                          ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %6
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB1
  %7 = phi i64 [ %11, %get.wi.properties ], [ 1, %SyncBB1 ]
  %add7 = add i64 %mul6, %6
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %7, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %6
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %8 = phi i64 [ %14, %get.wi.properties1 ], [ 1, %split.continue ]
  %add22 = add i64 %mul21, %6
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %8, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %1, i64 addrspace(1)* %arrayidx, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB1
  %9 = sext i32 %conv to i64
  %10 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 %9
  %11 = load i64* %10, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %12 = sext i32 %conv17 to i64
  %13 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 %12
  %14 = load i64* %13, align 8
  br label %split.continue2
}

define void @__get_global_id_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %6
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB
  %7 = phi i64 [ %15, %get.wi.properties ], [ 0, %SyncBB ]
  %add7 = add i64 %mul6, %6
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %7, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %6
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %8 = phi i64 [ %22, %get.wi.properties1 ], [ 0, %split.continue ]
  %add22 = add i64 %mul21, %6
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %8, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %6, i64 addrspace(1)* %arrayidx, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB
  %9 = sext i32 %conv to i64
  %10 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 %9
  %11 = load i64* %10, align 8
  %12 = sext i32 %conv to i64
  %13 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 %12
  %14 = load i64* %13, align 8
  %15 = add i64 %11, %14
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %16 = sext i32 %conv17 to i64
  %17 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 %16
  %18 = load i64* %17, align 8
  %19 = sext i32 %conv17 to i64
  %20 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 %19
  %21 = load i64* %20, align 8
  %22 = add i64 %18, %21
  br label %split.continue2
}

define void @__get_local_size_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  %2 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 0
  %3 = load i64* %2, align 8
  br label %SyncBB1

SyncBB1:                                          ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %8
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB1
  %9 = phi i64 [ %13, %get.wi.properties ], [ 1, %SyncBB1 ]
  %add7 = add i64 %mul6, %8
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %9, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %8
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %10 = phi i64 [ %16, %get.wi.properties1 ], [ 1, %split.continue ]
  %add22 = add i64 %mul21, %8
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %10, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %3, i64 addrspace(1)* %arrayidx, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB1
  %11 = sext i32 %conv to i64
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 %11
  %13 = load i64* %12, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %14 = sext i32 %conv17 to i64
  %15 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 3, i64 %14
  %16 = load i64* %15, align 8
  br label %split.continue2
}

define void @__get_local_id_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %2 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = add i64 %3, %5
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %6
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB
  %7 = phi i64 [ %13, %get.wi.properties ], [ 0, %SyncBB ]
  %add7 = add i64 %mul6, %6
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %7, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %6
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %8 = phi i64 [ %16, %get.wi.properties1 ], [ 0, %split.continue ]
  %add22 = add i64 %mul21, %6
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %8, i64 addrspace(1)* %arrayidx24, align 8
  %9 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %10 = load i64* %9, align 8
  store i64 %10, i64 addrspace(1)* %arrayidx, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB
  %11 = sext i32 %conv to i64
  %12 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 %11
  %13 = load i64* %12, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %14 = sext i32 %conv17 to i64
  %15 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 %14
  %16 = load i64* %15, align 8
  br label %split.continue2
}

define void @__get_num_groups_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  %2 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 0
  %3 = load i64* %2, align 8
  br label %SyncBB1

SyncBB1:                                          ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %8
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB1
  %9 = phi i64 [ %13, %get.wi.properties ], [ 1, %SyncBB1 ]
  %add7 = add i64 %mul6, %8
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %9, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %8
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %10 = phi i64 [ %16, %get.wi.properties1 ], [ 1, %split.continue ]
  %add22 = add i64 %mul21, %8
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %10, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %3, i64 addrspace(1)* %arrayidx, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB1
  %11 = sext i32 %conv to i64
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 %11
  %13 = load i64* %12, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %14 = sext i32 %conv17 to i64
  %15 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 4, i64 %14
  %16 = load i64* %15, align 8
  br label %split.continue2
}

define void @__get_group_id_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  %2 = load i64* %pWGId, align 8
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %3 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %6 = load i64* %5, align 8
  %7 = add i64 %4, %6
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %7
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB
  %8 = phi i64 [ %12, %get.wi.properties ], [ 0, %SyncBB ]
  %add7 = add i64 %mul6, %7
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %8, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %7
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %9 = phi i64 [ %15, %get.wi.properties1 ], [ 0, %split.continue ]
  %add22 = add i64 %mul21, %7
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %9, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %2, i64 addrspace(1)* %arrayidx, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB
  %10 = sext i32 %conv to i64
  %11 = getelementptr i64* %pWGId, i64 %10
  %12 = load i64* %11, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %13 = sext i32 %conv17 to i64
  %14 = getelementptr i64* %pWGId, i64 %13
  %15 = load i64* %14, align 8
  br label %split.continue2
}

define void @__get_global_offset_test_separated_args(i64 addrspace(1)* nocapture %pOutputs, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 2, i64 0
  %1 = load i64* %0, align 8
  %mul6 = shl i64 %1, 1
  %mul21 = mul i64 %1, 3
  %2 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 1, i64 0
  %3 = load i64* %2, align 8
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %4 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %5 = load i64* %4, align 8
  %6 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %7 = load i64* %6, align 8
  %8 = add i64 %5, %7
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %8
  %tmp2 = load i64 addrspace(1)* %arrayidx, align 8
  %conv = trunc i64 %tmp2 to i32
  %check.index.inbound = icmp ult i32 %conv, 3
  br i1 %check.index.inbound, label %get.wi.properties, label %split.continue

split.continue:                                   ; preds = %get.wi.properties, %SyncBB
  %9 = phi i64 [ %13, %get.wi.properties ], [ 0, %SyncBB ]
  %add7 = add i64 %mul6, %8
  %arrayidx9 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add7
  store i64 %9, i64 addrspace(1)* %arrayidx9, align 8
  %add13 = add i64 %1, %8
  %arrayidx15 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add13
  %tmp16 = load i64 addrspace(1)* %arrayidx15, align 8
  %conv17 = trunc i64 %tmp16 to i32
  %check.index.inbound3 = icmp ult i32 %conv17, 3
  br i1 %check.index.inbound3, label %get.wi.properties1, label %split.continue2

split.continue2:                                  ; preds = %get.wi.properties1, %split.continue
  %10 = phi i64 [ %16, %get.wi.properties1 ], [ 0, %split.continue ]
  %add22 = add i64 %mul21, %8
  %arrayidx24 = getelementptr inbounds i64 addrspace(1)* %pOutputs, i64 %add22
  store i64 %10, i64 addrspace(1)* %arrayidx24, align 8
  store i64 %3, i64 addrspace(1)* %arrayidx, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15, align 8
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %split.continue2
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %split.continue2
  ret void

get.wi.properties:                                ; preds = %SyncBB
  %11 = sext i32 %conv to i64
  %12 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 1, i64 %11
  %13 = load i64* %12, align 8
  br label %split.continue

get.wi.properties1:                               ; preds = %split.continue
  %14 = sext i32 %conv17 to i64
  %15 = getelementptr %struct.WorkDim* %pWorkDim, i64 0, i32 1, i64 %14
  %16 = load i64* %15, align 8
  br label %split.continue2
}

define void @get_local_id_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %16 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = add i64 %17, %19
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %20
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB.i
  %21 = phi i64 [ %27, %get.wi.properties.i ], [ 0, %SyncBB.i ]
  %add7.i = add i64 %mul6.i, %20
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %21, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %20
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %22 = phi i64 [ %30, %get.wi.properties1.i ], [ 0, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %20
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %22, i64 addrspace(1)* %arrayidx24.i, align 8
  %23 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %24 = load i64* %23, align 8
  store i64 %24, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_local_id_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

get.wi.properties.i:                              ; preds = %SyncBB.i
  %25 = sext i32 %conv.i to i64
  %26 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 %25
  %27 = load i64* %26, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %28 = sext i32 %conv17.i to i64
  %29 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 %28
  %30 = load i64* %29, align 8
  br label %split.continue2.i

__get_local_id_test_separated_args.exit:          ; preds = %split.continue2.i
  ret void
}

define void @get_global_id_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %16 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = add i64 %17, %19
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %20
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB.i
  %21 = phi i64 [ %29, %get.wi.properties.i ], [ 0, %SyncBB.i ]
  %add7.i = add i64 %mul6.i, %20
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %21, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %20
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %22 = phi i64 [ %36, %get.wi.properties1.i ], [ 0, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %20
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %22, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %20, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_global_id_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

get.wi.properties.i:                              ; preds = %SyncBB.i
  %23 = sext i32 %conv.i to i64
  %24 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 %23
  %25 = load i64* %24, align 8
  %26 = sext i32 %conv.i to i64
  %27 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 %26
  %28 = load i64* %27, align 8
  %29 = add i64 %25, %28
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %30 = sext i32 %conv17.i to i64
  %31 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 %30
  %32 = load i64* %31, align 8
  %33 = sext i32 %conv17.i to i64
  %34 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 %33
  %35 = load i64* %34, align 8
  %36 = add i64 %32, %35
  br label %split.continue2.i

__get_global_id_test_separated_args.exit:         ; preds = %split.continue2.i
  ret void
}

define void @get_group_id_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 24
  %6 = bitcast i8* %5 to i64**
  %7 = load i64** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 32
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %18 = load i64* %17, align 8
  %mul6.i = shl i64 %18, 1
  %mul21.i = mul i64 %18, 3
  %19 = load i64* %7, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %20 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %23 = load i64* %22, align 8
  %24 = add i64 %21, %23
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %24
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB.i
  %25 = phi i64 [ %29, %get.wi.properties.i ], [ 0, %SyncBB.i ]
  %add7.i = add i64 %mul6.i, %24
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %25, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %18, %24
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %26 = phi i64 [ %32, %get.wi.properties1.i ], [ 0, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %24
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %26, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %19, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_group_id_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

get.wi.properties.i:                              ; preds = %SyncBB.i
  %27 = sext i32 %conv.i to i64
  %28 = getelementptr i64* %7, i64 %27
  %29 = load i64* %28, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %30 = sext i32 %conv17.i to i64
  %31 = getelementptr i64* %7, i64 %30
  %32 = load i64* %31, align 8
  br label %split.continue2.i

__get_group_id_test_separated_args.exit:          ; preds = %split.continue2.i
  ret void
}

define void @get_global_offset_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  %16 = getelementptr %struct.WorkDim* %4, i64 0, i32 1, i64 0
  %17 = load i64* %16, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %18 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = add i64 %19, %21
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %22
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB.i
  %23 = phi i64 [ %27, %get.wi.properties.i ], [ 0, %SyncBB.i ]
  %add7.i = add i64 %mul6.i, %22
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %23, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %22
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %24 = phi i64 [ %30, %get.wi.properties1.i ], [ 0, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %22
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %24, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %17, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 0, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_global_offset_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

get.wi.properties.i:                              ; preds = %SyncBB.i
  %25 = sext i32 %conv.i to i64
  %26 = getelementptr %struct.WorkDim* %4, i64 0, i32 1, i64 %25
  %27 = load i64* %26, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %28 = sext i32 %conv17.i to i64
  %29 = getelementptr %struct.WorkDim* %4, i64 0, i32 1, i64 %28
  %30 = load i64* %29, align 8
  br label %split.continue2.i

__get_global_offset_test_separated_args.exit:     ; preds = %split.continue2.i
  ret void
}

define void @get_local_size_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  %16 = getelementptr %struct.WorkDim* %4, i64 0, i32 3, i64 0
  %17 = load i64* %16, align 8
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %18 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = add i64 %19, %21
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %22
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB1.i
  %23 = phi i64 [ %27, %get.wi.properties.i ], [ 1, %SyncBB1.i ]
  %add7.i = add i64 %mul6.i, %22
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %23, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %22
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %24 = phi i64 [ %30, %get.wi.properties1.i ], [ 1, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %22
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %24, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %17, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_local_size_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

get.wi.properties.i:                              ; preds = %SyncBB1.i
  %25 = sext i32 %conv.i to i64
  %26 = getelementptr %struct.WorkDim* %4, i64 0, i32 3, i64 %25
  %27 = load i64* %26, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %28 = sext i32 %conv17.i to i64
  %29 = getelementptr %struct.WorkDim* %4, i64 0, i32 3, i64 %28
  %30 = load i64* %29, align 8
  br label %split.continue2.i

__get_local_size_test_separated_args.exit:        ; preds = %split.continue2.i
  ret void
}

define void @get_num_groups_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  %16 = getelementptr %struct.WorkDim* %4, i64 0, i32 4, i64 0
  %17 = load i64* %16, align 8
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %18 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %21 = load i64* %20, align 8
  %22 = add i64 %19, %21
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %22
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB1.i
  %23 = phi i64 [ %27, %get.wi.properties.i ], [ 1, %SyncBB1.i ]
  %add7.i = add i64 %mul6.i, %22
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %23, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %22
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %24 = phi i64 [ %30, %get.wi.properties1.i ], [ 1, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %22
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %24, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %17, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_num_groups_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

get.wi.properties.i:                              ; preds = %SyncBB1.i
  %25 = sext i32 %conv.i to i64
  %26 = getelementptr %struct.WorkDim* %4, i64 0, i32 4, i64 %25
  %27 = load i64* %26, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %28 = sext i32 %conv17.i to i64
  %29 = getelementptr %struct.WorkDim* %4, i64 0, i32 4, i64 %28
  %30 = load i64* %29, align 8
  br label %split.continue2.i

__get_num_groups_test_separated_args.exit:        ; preds = %split.continue2.i
  ret void
}

define void @get_global_size_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i64 addrspace(1)**
  %1 = load i64 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 16
  %3 = bitcast i8* %2 to %struct.WorkDim**
  %4 = load %struct.WorkDim** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 32
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 56
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 0
  %15 = load i64* %14, align 8
  %mul6.i = shl i64 %15, 1
  %mul21.i = mul i64 %15, 3
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %16 = getelementptr %struct.PaddedDimId* %10, i64 %CurrWI..0.i, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = getelementptr %struct.PaddedDimId* %7, i64 0, i32 0, i64 0
  %19 = load i64* %18, align 8
  %20 = add i64 %17, %19
  %arrayidx.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %20
  %tmp2.i = load i64 addrspace(1)* %arrayidx.i, align 8
  %conv.i = trunc i64 %tmp2.i to i32
  %check.index.inbound.i = icmp ult i32 %conv.i, 3
  br i1 %check.index.inbound.i, label %get.wi.properties.i, label %split.continue.i

split.continue.i:                                 ; preds = %get.wi.properties.i, %SyncBB1.i
  %21 = phi i64 [ %25, %get.wi.properties.i ], [ 1, %SyncBB1.i ]
  %add7.i = add i64 %mul6.i, %20
  %arrayidx9.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add7.i
  store i64 %21, i64 addrspace(1)* %arrayidx9.i, align 8
  %add13.i = add i64 %15, %20
  %arrayidx15.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add13.i
  %tmp16.i = load i64 addrspace(1)* %arrayidx15.i, align 8
  %conv17.i = trunc i64 %tmp16.i to i32
  %check.index.inbound3.i = icmp ult i32 %conv17.i, 3
  br i1 %check.index.inbound3.i, label %get.wi.properties1.i, label %split.continue2.i

split.continue2.i:                                ; preds = %get.wi.properties1.i, %split.continue.i
  %22 = phi i64 [ %28, %get.wi.properties1.i ], [ 1, %split.continue.i ]
  %add22.i = add i64 %mul21.i, %20
  %arrayidx24.i = getelementptr inbounds i64 addrspace(1)* %1, i64 %add22.i
  store i64 %22, i64 addrspace(1)* %arrayidx24.i, align 8
  store i64 %15, i64 addrspace(1)* %arrayidx.i, align 8
  store i64 1, i64 addrspace(1)* %arrayidx15.i, align 8
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__get_global_size_test_separated_args.exit

thenBB.i:                                         ; preds = %split.continue2.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

get.wi.properties.i:                              ; preds = %SyncBB1.i
  %23 = sext i32 %conv.i to i64
  %24 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 %23
  %25 = load i64* %24, align 8
  br label %split.continue.i

get.wi.properties1.i:                             ; preds = %split.continue.i
  %26 = sext i32 %conv17.i to i64
  %27 = getelementptr %struct.WorkDim* %4, i64 0, i32 2, i64 %26
  %28 = load i64* %27, align 8
  br label %split.continue2.i

__get_global_size_test_separated_args.exit:       ; preds = %split.continue2.i
  ret void
}

!opencl.kernels = !{!0, !6, !7, !8, !9, !10, !11}
!opencl.build.options = !{}

!0 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_global_size_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_global_size_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_global_size_test}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"size_t*"}
!5 = metadata !{metadata !"pOutputs"}
!6 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_global_id_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_global_id_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_global_id_test}
!7 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_local_size_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_local_size_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_local_size_test}
!8 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_local_id_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_local_id_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_local_id_test}
!9 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_num_groups_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_num_groups_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_num_groups_test}
!10 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_group_id_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_group_id_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_group_id_test}
!11 = metadata !{void (i64 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__get_global_offset_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"size_t __attribute__((address_space(1))) *", metadata !"opencl_get_global_offset_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @get_global_offset_test}
