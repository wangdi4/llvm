; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }
%struct._TEST_STRUCT = type { i8, i16, i32, float }
%union._TEST_UNION = type { i32 }

declare void @__test_original(%struct._TEST_STRUCT* nocapture byval, %union._TEST_UNION* nocapture byval, %struct._TEST_STRUCT addrspace(1)* nocapture) nounwind

declare i64 @get_global_id(i32) readnone

declare void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* nocapture, i8* nocapture, i64, i32, i1) nounwind

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__test_separated_args(%struct._TEST_STRUCT* nocapture byval %src1, %union._TEST_UNION* nocapture byval %src2, %struct._TEST_STRUCT addrspace(1)* nocapture %dst, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  %tmp3 = getelementptr inbounds %struct._TEST_STRUCT* %src1, i64 0, i32 0
  %tmp4 = getelementptr inbounds %union._TEST_UNION* %src2, i64 0, i32 0
  %tmp5 = load i32* %tmp4, align 4
  br label %SyncBB

SyncBB:                                           ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ 0, %entry ], [ %"CurrWI++", %thenBB ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %tmp2 = getelementptr inbounds %struct._TEST_STRUCT addrspace(1)* %dst, i64 %4, i32 0
  tail call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %tmp2, i8* %tmp3, i64 12, i32 4, i1 false)
  %tmp9 = getelementptr inbounds %struct._TEST_STRUCT addrspace(1)* %dst, i64 %4, i32 2
  %tmp10 = load i32 addrspace(1)* %tmp9, align 4
  %add = add nsw i32 %tmp10, %tmp5
  store i32 %add, i32 addrspace(1)* %tmp9, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %SyncBB
  ret void
}

define void @test(i8* %pBuffer) {
entry:
  %src1 = alloca %struct._TEST_STRUCT, align 8
  %0 = getelementptr i8* %pBuffer, i64 12
  %1 = getelementptr i8* %pBuffer, i64 16
  %2 = bitcast i8* %1 to %struct._TEST_STRUCT addrspace(1)**
  %3 = load %struct._TEST_STRUCT addrspace(1)** %2, align 8
  %4 = getelementptr i8* %pBuffer, i64 48
  %5 = bitcast i8* %4 to %struct.PaddedDimId**
  %6 = load %struct.PaddedDimId** %5, align 8
  %7 = getelementptr i8* %pBuffer, i64 56
  %8 = bitcast i8* %7 to %struct.PaddedDimId**
  %9 = load %struct.PaddedDimId** %8, align 8
  %10 = getelementptr i8* %pBuffer, i64 72
  %11 = bitcast i8* %10 to i64*
  %12 = load i64* %11, align 8
  %tmp = getelementptr inbounds %struct._TEST_STRUCT* %src1, i64 0, i32 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %tmp, i8* %pBuffer, i64 12, i32 1, i1 false)
  %13 = bitcast i8* %0 to i32*
  %tmp4 = load i32* %13, align 1
  %tmp3.i = getelementptr inbounds %struct._TEST_STRUCT* %src1, i64 0, i32 0
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ 0, %entry ], [ %"CurrWI++.i", %thenBB.i ]
  %14 = getelementptr %struct.PaddedDimId* %9, i64 %CurrWI..0.i, i32 0, i64 0
  %15 = load i64* %14, align 8
  %16 = getelementptr %struct.PaddedDimId* %6, i64 0, i32 0, i64 0
  %17 = load i64* %16, align 8
  %18 = add i64 %15, %17
  %tmp2.i = getelementptr inbounds %struct._TEST_STRUCT addrspace(1)* %3, i64 %18, i32 0
  call void @llvm.memcpy.p1i8.p0i8.i64(i8 addrspace(1)* %tmp2.i, i8* %tmp3.i, i64 12, i32 4, i1 false) nounwind
  %tmp9.i = getelementptr inbounds %struct._TEST_STRUCT addrspace(1)* %3, i64 %18, i32 2
  %tmp10.i = load i32 addrspace(1)* %tmp9.i, align 4
  %add.i = add nsw i32 %tmp10.i, %tmp4
  store i32 %add.i, i32 addrspace(1)* %tmp9.i, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %12
  br i1 %check.WI.iter.i, label %thenBB.i, label %__test_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__test_separated_args.exit:                       ; preds = %SyncBB.i
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture, i64, i32, i1) nounwind

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (%struct._TEST_STRUCT*, %union._TEST_UNION*, %struct._TEST_STRUCT addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__test_separated_args, metadata !1, metadata !1, metadata !"", metadata !"TEST_STRUCT, TEST_UNION, TEST_STRUCT __attribute__((address_space(1))) *", metadata !"opencl_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @test}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0, i32 0, i32 1}
!3 = metadata !{i32 3, i32 3, i32 3}
!4 = metadata !{metadata !"TEST_STRUCT", metadata !"TEST_UNION", metadata !"TEST_STRUCT*"}
!5 = metadata !{metadata !"src1", metadata !"src2", metadata !"dst"}
