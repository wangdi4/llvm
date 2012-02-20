; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__k2_original(i8 addrspace(1)* nocapture, i8 signext) nounwind

declare i64 @get_global_id(i32) readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__k2_separated_args(i8 addrspace(1)* nocapture %p, i8 signext %id, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %conv3 = zext i8 %id to i32
  %add = shl i32 %conv3, 24
  %sext = add i32 %add, 1073741824
  %conv2 = ashr i32 %sext, 24
  %add13 = add i8 %id, 32
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %p, i64 %4
  %tmp4 = load i8 addrspace(1)* %arrayidx, align 1
  %conv5 = sext i8 %tmp4 to i32
  %cmp = icmp eq i32 %conv2, %conv5
  %storemerge = select i1 %cmp, i8 %id, i8 %add13
  store i8 %storemerge, i8 addrspace(1)* %arrayidx, align 1
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB5:                                          ; preds = %SyncBB
  ret void
}

define void @k2(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i8 addrspace(1)**
  %1 = load i8 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = load i8* %2, align 1
  %4 = getelementptr i8* %pBuffer, i64 40
  %5 = bitcast i8* %4 to %struct.PaddedDimId**
  %6 = load %struct.PaddedDimId** %5, align 8
  %7 = getelementptr i8* %pBuffer, i64 48
  %8 = bitcast i8* %7 to %struct.PaddedDimId**
  %9 = load %struct.PaddedDimId** %8, align 8
  %10 = getelementptr i8* %pBuffer, i64 64
  %11 = bitcast i8* %10 to i64*
  %12 = load i64* %11, align 8
  %conv3.i = zext i8 %3 to i32
  %add.i = shl i32 %conv3.i, 24
  %sext.i = add i32 %add.i, 1073741824
  %conv2.i = ashr i32 %sext.i, 24
  %add13.i = add i8 %3, 32
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %13 = getelementptr %struct.PaddedDimId* %9, i64 %CurrWI..0.i, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = getelementptr %struct.PaddedDimId* %6, i64 0, i32 0, i64 0
  %16 = load i64* %15, align 8
  %17 = add i64 %14, %16
  %arrayidx.i = getelementptr inbounds i8 addrspace(1)* %1, i64 %17
  %tmp4.i = load i8 addrspace(1)* %arrayidx.i, align 1
  %conv5.i = sext i8 %tmp4.i to i32
  %cmp.i = icmp eq i32 %conv2.i, %conv5.i
  %storemerge.i = select i1 %cmp.i, i8 %3, i8 %add13.i
  store i8 %storemerge.i, i8 addrspace(1)* %arrayidx.i, align 1
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %12
  br i1 %check.WI.iter.i, label %thenBB.i, label %__k2_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__k2_separated_args.exit:                         ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i8 addrspace(1)*, i8, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__k2_separated_args, metadata !1, metadata !1, metadata !"", metadata !"char __attribute__((address_space(1))) *, char", metadata !"opencl_k2_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @k2}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 0}
!3 = metadata !{i32 3, i32 3}
!4 = metadata !{metadata !"char*", metadata !"char"}
!5 = metadata !{metadata !"p", metadata !"id"}
