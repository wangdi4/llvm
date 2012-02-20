; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

@opencl_barrier_test_local_tmp.4.5 = internal addrspace(3) global i32 0

declare void @__barrier_test_original(i32 addrspace(1)* nocapture, i32 addrspace(3)* nocapture) nounwind

declare i64 @get_global_id(i32) readnone

declare void @barrier(i64)

declare void @dummybarrier.()

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__barrier_test_separated_args(i32 addrspace(1)* nocapture %res, i32 addrspace(3)* nocapture %tmp2, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to i32 addrspace(3)*
  br label %SyncBB6

SyncBB6:                                          ; preds = %entry, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %entry ]
  %CurrSBIndex..0 = phi i64 [ %"loadedCurrSB+Stride", %thenBB ], [ 0, %entry ]
  %1 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %2 = load i64* %1, align 8
  %3 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %4 = load i64* %3, align 8
  %5 = add i64 %2, %4
  %conv = trunc i64 %5 to i32
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i32*
  store i32 %conv, i32* %CastToValueType, align 4
  store i32 %conv, i32 addrspace(3)* %tmp2, align 4
  store i32 %conv, i32 addrspace(3)* %0, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %SyncBB6
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 4
  br label %SyncBB6

SyncBB:                                           ; preds = %SyncBB6, %thenBB9
  %CurrWI..1 = phi i64 [ %"CurrWI++13", %thenBB9 ], [ 0, %SyncBB6 ]
  %CurrSBIndex..1 = phi i64 [ %"loadedCurrSB+Stride15", %thenBB9 ], [ 0, %SyncBB6 ]
  %tmp6 = load i32 addrspace(3)* %0, align 4
  %"&pSB[currWI].offset4" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..1
  %CastToValueType5 = bitcast i8* %"&pSB[currWI].offset4" to i32*
  %loadedValue = load i32* %CastToValueType5, align 4
  %idxprom = sext i32 %loadedValue to i64
  %arrayidx9 = getelementptr inbounds i32 addrspace(1)* %res, i64 %idxprom
  store i32 %tmp6, i32 addrspace(1)* %arrayidx9, align 4
  %check.WI.iter12 = icmp ult i64 %CurrWI..1, %iterCount
  br i1 %check.WI.iter12, label %thenBB9, label %SyncBB7

thenBB9:                                          ; preds = %SyncBB
  %"CurrWI++13" = add nuw i64 %CurrWI..1, 1
  %"loadedCurrSB+Stride15" = add nuw i64 %CurrSBIndex..1, 4
  br label %SyncBB

SyncBB7:                                          ; preds = %SyncBB
  ret void
}

define void @barrier_test(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i32 addrspace(3)**
  %4 = load i32 addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i8 addrspace(3)**
  %7 = load i8 addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 40
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 48
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 64
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  %17 = getelementptr i8* %pBuffer, i64 72
  %18 = bitcast i8* %17 to i8**
  %19 = load i8** %18, align 8
  %20 = bitcast i8 addrspace(3)* %7 to i32 addrspace(3)*
  br label %SyncBB6.i

SyncBB6.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %CurrSBIndex..0.i = phi i64 [ %"loadedCurrSB+Stride.i", %thenBB.i ], [ 0, %entry ]
  %21 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %22 = load i64* %21, align 8
  %23 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %24 = load i64* %23, align 8
  %25 = add i64 %22, %24
  %conv.i = trunc i64 %25 to i32
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i32*
  store i32 %conv.i, i32* %CastToValueType.i, align 4
  store i32 %conv.i, i32 addrspace(3)* %4, align 4
  store i32 %conv.i, i32 addrspace(3)* %20, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %SyncBB.i

thenBB.i:                                         ; preds = %SyncBB6.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 4
  br label %SyncBB6.i

SyncBB.i:                                         ; preds = %thenBB9.i, %SyncBB6.i
  %CurrWI..1.i = phi i64 [ %"CurrWI++13.i", %thenBB9.i ], [ 0, %SyncBB6.i ]
  %CurrSBIndex..1.i = phi i64 [ %"loadedCurrSB+Stride15.i", %thenBB9.i ], [ 0, %SyncBB6.i ]
  %tmp6.i = load i32 addrspace(3)* %20, align 4
  %"&pSB[currWI].offset4.i" = getelementptr inbounds i8* %19, i64 %CurrSBIndex..1.i
  %CastToValueType5.i = bitcast i8* %"&pSB[currWI].offset4.i" to i32*
  %loadedValue.i = load i32* %CastToValueType5.i, align 4
  %idxprom.i = sext i32 %loadedValue.i to i64
  %arrayidx9.i = getelementptr inbounds i32 addrspace(1)* %1, i64 %idxprom.i
  store i32 %tmp6.i, i32 addrspace(1)* %arrayidx9.i, align 4
  %check.WI.iter12.i = icmp ult i64 %CurrWI..1.i, %16
  br i1 %check.WI.iter12.i, label %thenBB9.i, label %__barrier_test_separated_args.exit

thenBB9.i:                                        ; preds = %SyncBB.i
  %"CurrWI++13.i" = add nuw i64 %CurrWI..1.i, 1
  %"loadedCurrSB+Stride15.i" = add nuw i64 %CurrSBIndex..1.i, 4
  br label %SyncBB.i

__barrier_test_separated_args.exit:               ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}
!opencl_barrier_test_locals_anchor = !{!6}
!opencl.build.options = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__barrier_test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, int __attribute__((address_space(3))) *", metadata !"opencl_barrier_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @barrier_test}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 3}
!3 = metadata !{i32 3, i32 3}
!4 = metadata !{metadata !"int*", metadata !"int*"}
!5 = metadata !{metadata !"res", metadata !"tmp2"}
!6 = metadata !{metadata !"opencl_barrier_test_local_tmp"}
