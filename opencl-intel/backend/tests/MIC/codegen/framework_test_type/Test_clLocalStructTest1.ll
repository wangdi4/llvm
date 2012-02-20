; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }
%struct._AABB = type { [3 x i32], [3 x i32] }

@opencl_Foo_local_tileBB = internal addrspace(3) global %struct._AABB zeroinitializer, align 4

declare void @__Foo_original(%struct._AABB addrspace(1)*, %struct._AABB addrspace(3)* nocapture) nounwind

declare void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* nocapture, i8 addrspace(3)* nocapture, i64, i32, i1) nounwind

declare i64 @_Z21async_work_group_copyPU3AS1cPKU3AS3cmm(i8 addrspace(1)*, i8 addrspace(3)*, i64, i64)

declare void @_Z17wait_group_eventsiPm(i32, i64*)

declare void @llvm.memcpy.p1i8.p3i8.i64(i8 addrspace(1)* nocapture, i8 addrspace(3)* nocapture, i64, i32, i1) nounwind

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__Foo_separated_args(%struct._AABB addrspace(1)* %p, %struct._AABB addrspace(3)* nocapture %lcl, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %0 = bitcast i8 addrspace(3)* %pLocalMem to i32 addrspace(3)*
  %1 = getelementptr i8 addrspace(3)* %pLocalMem, i64 4
  %2 = bitcast i8 addrspace(3)* %1 to i32 addrspace(3)*
  %3 = getelementptr i8 addrspace(3)* %pLocalMem, i64 8
  %4 = bitcast i8 addrspace(3)* %3 to i32 addrspace(3)*
  %5 = getelementptr i8 addrspace(3)* %pLocalMem, i64 12
  %6 = bitcast i8 addrspace(3)* %5 to i32 addrspace(3)*
  %7 = getelementptr i8 addrspace(3)* %pLocalMem, i64 16
  %8 = bitcast i8 addrspace(3)* %7 to i32 addrspace(3)*
  %9 = getelementptr i8 addrspace(3)* %pLocalMem, i64 20
  %10 = bitcast i8 addrspace(3)* %9 to i32 addrspace(3)*
  %tmp1 = bitcast %struct._AABB addrspace(3)* %lcl to i8 addrspace(3)*
  %11 = bitcast %struct._AABB addrspace(1)* %p to i8 addrspace(1)*
  %arrayidx6 = getelementptr inbounds %struct._AABB addrspace(1)* %p, i64 1
  %tmp8 = bitcast %struct._AABB addrspace(1)* %arrayidx6 to i8 addrspace(1)*
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %CurrSBIndex..0 = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride", %thenBB ]
  store i32 -1, i32 addrspace(3)* %0, align 4
  store i32 -2, i32 addrspace(3)* %2, align 4
  store i32 -3, i32 addrspace(3)* %4, align 4
  store i32 1, i32 addrspace(3)* %6, align 4
  store i32 2, i32 addrspace(3)* %8, align 4
  store i32 3, i32 addrspace(3)* %10, align 4
  call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* %tmp1, i8 addrspace(3)* %pLocalMem, i64 24, i32 4, i1 false)
  %12 = bitcast i8 addrspace(1)* %11 to i8*
  %13 = bitcast i8 addrspace(3)* %pLocalMem to i8*
  %14 = call i64 @lasync_wg_copy_l2g(i8* %12, i8* %13, i64 24, i64 0, i64 1, i64* %contextpointer) nounwind
  %"&pSB[currWI].offset3" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType4 = bitcast i8* %"&pSB[currWI].offset3" to i64*
  store i64 %14, i64* %CastToValueType4, align 8
  %"&pSB[currWI].offset" = getelementptr inbounds i8* %pSpecialBuf, i64 %CurrSBIndex..0
  %CastToValueType = bitcast i8* %"&pSB[currWI].offset" to i64*
  call void @lwait_group_events(i32 1, i64* %CastToValueType, i64* %contextpointer) nounwind
  call void @llvm.memcpy.p1i8.p3i8.i64(i8 addrspace(1)* %tmp8, i8 addrspace(3)* %tmp1, i64 24, i32 4, i1 false)
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB5

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  %"loadedCurrSB+Stride" = add nuw i64 %CurrSBIndex..0, 8
  br label %SyncBB

SyncBB5:                                          ; preds = %SyncBB
  ret void
}

declare i64 @lasync_wg_copy_l2g(i8*, i8*, i64, i64, i64, i64*)

declare i64 @lasync_wg_copy_g2l(i8*, i8*, i64, i64, i64, i64*)

declare i64 @lasync_wg_copy_strided_l2g(i8*, i8*, i64, i64, i64, i64, i64*)

declare i64 @lasync_wg_copy_strided_g2l(i8*, i8*, i64, i64, i64, i64, i64*)

declare void @lwait_group_events(i32, i64*, i64*)

define void @Foo(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to %struct._AABB addrspace(1)**
  %1 = load %struct._AABB addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to %struct._AABB addrspace(3)**
  %4 = load %struct._AABB addrspace(3)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i8 addrspace(3)**
  %7 = load i8 addrspace(3)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 56
  %9 = bitcast i8* %8 to i64**
  %10 = load i64** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 64
  %12 = bitcast i8* %11 to i64*
  %13 = load i64* %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 72
  %15 = bitcast i8* %14 to i8**
  %16 = load i8** %15, align 8
  %17 = bitcast i8 addrspace(3)* %7 to i32 addrspace(3)*
  %18 = getelementptr i8 addrspace(3)* %7, i64 4
  %19 = bitcast i8 addrspace(3)* %18 to i32 addrspace(3)*
  %20 = getelementptr i8 addrspace(3)* %7, i64 8
  %21 = bitcast i8 addrspace(3)* %20 to i32 addrspace(3)*
  %22 = getelementptr i8 addrspace(3)* %7, i64 12
  %23 = bitcast i8 addrspace(3)* %22 to i32 addrspace(3)*
  %24 = getelementptr i8 addrspace(3)* %7, i64 16
  %25 = bitcast i8 addrspace(3)* %24 to i32 addrspace(3)*
  %26 = getelementptr i8 addrspace(3)* %7, i64 20
  %27 = bitcast i8 addrspace(3)* %26 to i32 addrspace(3)*
  %tmp1.i = bitcast %struct._AABB addrspace(3)* %4 to i8 addrspace(3)*
  %28 = bitcast %struct._AABB addrspace(1)* %1 to i8 addrspace(1)*
  %arrayidx6.i = getelementptr inbounds %struct._AABB addrspace(1)* %1, i64 1
  %tmp8.i = bitcast %struct._AABB addrspace(1)* %arrayidx6.i to i8 addrspace(1)*
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %CurrSBIndex..0.i = phi i64 [ 0, %entry ], [ %"loadedCurrSB+Stride.i", %thenBB.i ]
  store i32 -1, i32 addrspace(3)* %17, align 4
  store i32 -2, i32 addrspace(3)* %19, align 4
  store i32 -3, i32 addrspace(3)* %21, align 4
  store i32 1, i32 addrspace(3)* %23, align 4
  store i32 2, i32 addrspace(3)* %25, align 4
  store i32 3, i32 addrspace(3)* %27, align 4
  call void @llvm.memcpy.p3i8.p3i8.i64(i8 addrspace(3)* %tmp1.i, i8 addrspace(3)* %7, i64 24, i32 4, i1 false) nounwind
  %29 = bitcast i8 addrspace(1)* %28 to i8*
  %30 = bitcast i8 addrspace(3)* %7 to i8*
  %31 = call i64 @lasync_wg_copy_l2g(i8* %29, i8* %30, i64 24, i64 0, i64 1, i64* %10) nounwind
  %"&pSB[currWI].offset3.i" = getelementptr inbounds i8* %16, i64 %CurrSBIndex..0.i
  %CastToValueType4.i = bitcast i8* %"&pSB[currWI].offset3.i" to i64*
  store i64 %31, i64* %CastToValueType4.i, align 8
  %"&pSB[currWI].offset.i" = getelementptr inbounds i8* %16, i64 %CurrSBIndex..0.i
  %CastToValueType.i = bitcast i8* %"&pSB[currWI].offset.i" to i64*
  call void @lwait_group_events(i32 1, i64* %CastToValueType.i, i64* %10) nounwind
  call void @llvm.memcpy.p1i8.p3i8.i64(i8 addrspace(1)* %tmp8.i, i8 addrspace(3)* %tmp1.i, i64 24, i32 4, i1 false) nounwind
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %13
  br i1 %check.WI.iter.i, label %thenBB.i, label %__Foo_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  %"loadedCurrSB+Stride.i" = add nuw i64 %CurrSBIndex..0.i, 8
  br label %SyncBB.i

__Foo_separated_args.exit:                        ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}
!opencl_Foo_locals_anchor = !{!6}
!opencl.build.options = !{}

!0 = metadata !{void (%struct._AABB addrspace(1)*, %struct._AABB addrspace(3)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__Foo_separated_args, metadata !1, metadata !1, metadata !"", metadata !"AABB __attribute__((address_space(1))) *, AABB __attribute__((address_space(3))) *", metadata !"opencl_Foo_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @Foo}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 3}
!3 = metadata !{i32 3, i32 3}
!4 = metadata !{metadata !"AABB*", metadata !"AABB*"}
!5 = metadata !{metadata !"p", metadata !"lcl"}
!6 = metadata !{metadata !"opencl_Foo_local_tileBB"}
