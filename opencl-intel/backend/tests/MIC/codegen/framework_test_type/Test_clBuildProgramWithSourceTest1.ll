; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }
%struct._image2d_t = type opaque

declare void @__test_kernel_original(<16 x i8> addrspace(1)* nocapture, i8 addrspace(1)* nocapture, i8 addrspace(1)* nocapture, %struct._image2d_t* nocapture) nounwind

declare i64 @get_global_id(i32) readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__test_kernel_separated_args(<16 x i8> addrspace(1)* nocapture %pBuff0, i8 addrspace(1)* nocapture %pBuff1, i8 addrspace(1)* nocapture %pBuff2, %struct._image2d_t* nocapture %test_image, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  br label %SyncBB1

SyncBB1:                                          ; preds = %entry, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %entry ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %pBuff1, i64 %4
  %tmp2 = load i8 addrspace(1)* %arrayidx, align 1
  %tobool = icmp eq i8 %tmp2, 0
  br i1 %tobool, label %cond.false, label %cond.true

cond.true:                                        ; preds = %SyncBB1
  %arrayidx5 = getelementptr inbounds <16 x i8> addrspace(1)* %pBuff0, i64 %4
  %tmp6 = load <16 x i8> addrspace(1)* %arrayidx5, align 16
  br label %cond.end

cond.false:                                       ; preds = %SyncBB1
  %arrayidx9 = getelementptr inbounds i8 addrspace(1)* %pBuff2, i64 %4
  %tmp10 = load i8 addrspace(1)* %arrayidx9, align 1
  %tmp11 = insertelement <16 x i8> undef, i8 %tmp10, i32 0
  %splat = shufflevector <16 x i8> %tmp11, <16 x i8> undef, <16 x i32> zeroinitializer
  %arrayidx14.pre = getelementptr inbounds <16 x i8> addrspace(1)* %pBuff0, i64 %4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %arrayidx14.pre-phi = phi <16 x i8> addrspace(1)* [ %arrayidx14.pre, %cond.false ], [ %arrayidx5, %cond.true ]
  %cond = phi <16 x i8> [ %splat, %cond.false ], [ %tmp6, %cond.true ]
  store <16 x i8> %cond, <16 x i8> addrspace(1)* %arrayidx14.pre-phi, align 16
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB

thenBB:                                           ; preds = %cond.end
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB1

SyncBB:                                           ; preds = %cond.end
  ret void
}

define void @test_kernel(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <16 x i8> addrspace(1)**
  %1 = load <16 x i8> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to i8 addrspace(1)**
  %4 = load i8 addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to i8 addrspace(1)**
  %7 = load i8 addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 56
  %9 = bitcast i8* %8 to %struct.PaddedDimId**
  %10 = load %struct.PaddedDimId** %9, align 8
  %11 = getelementptr i8* %pBuffer, i64 64
  %12 = bitcast i8* %11 to %struct.PaddedDimId**
  %13 = load %struct.PaddedDimId** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 80
  %15 = bitcast i8* %14 to i64*
  %16 = load i64* %15, align 8
  br label %SyncBB1.i

SyncBB1.i:                                        ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %17 = getelementptr %struct.PaddedDimId* %13, i64 %CurrWI..0.i, i32 0, i64 0
  %18 = load i64* %17, align 8
  %19 = getelementptr %struct.PaddedDimId* %10, i64 0, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = add i64 %18, %20
  %arrayidx.i = getelementptr inbounds i8 addrspace(1)* %4, i64 %21
  %tmp2.i = load i8 addrspace(1)* %arrayidx.i, align 1
  %tobool.i = icmp eq i8 %tmp2.i, 0
  br i1 %tobool.i, label %cond.false.i, label %cond.true.i

cond.true.i:                                      ; preds = %SyncBB1.i
  %arrayidx5.i = getelementptr inbounds <16 x i8> addrspace(1)* %1, i64 %21
  %tmp6.i = load <16 x i8> addrspace(1)* %arrayidx5.i, align 16
  br label %cond.end.i

cond.false.i:                                     ; preds = %SyncBB1.i
  %arrayidx9.i = getelementptr inbounds i8 addrspace(1)* %7, i64 %21
  %tmp10.i = load i8 addrspace(1)* %arrayidx9.i, align 1
  %tmp11.i = insertelement <16 x i8> undef, i8 %tmp10.i, i32 0
  %splat.i = shufflevector <16 x i8> %tmp11.i, <16 x i8> undef, <16 x i32> zeroinitializer
  %arrayidx14.pre.i = getelementptr inbounds <16 x i8> addrspace(1)* %1, i64 %21
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.false.i, %cond.true.i
  %arrayidx14.pre-phi.i = phi <16 x i8> addrspace(1)* [ %arrayidx14.pre.i, %cond.false.i ], [ %arrayidx5.i, %cond.true.i ]
  %cond.i = phi <16 x i8> [ %splat.i, %cond.false.i ], [ %tmp6.i, %cond.true.i ]
  store <16 x i8> %cond.i, <16 x i8> addrspace(1)* %arrayidx14.pre-phi.i, align 16
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %16
  br i1 %check.WI.iter.i, label %thenBB.i, label %__test_kernel_separated_args.exit

thenBB.i:                                         ; preds = %cond.end.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB1.i

__test_kernel_separated_args.exit:                ; preds = %cond.end.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (<16 x i8> addrspace(1)*, i8 addrspace(1)*, i8 addrspace(1)*, %struct._image2d_t*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__test_kernel_separated_args, metadata !1, metadata !1, metadata !"", metadata !"char16 __attribute__((address_space(1))) *, char __attribute__((address_space(1))) *, char __attribute__((address_space(1))) *, __rd image2d_t", metadata !"opencl_test_kernel_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @test_kernel}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 1, i32 1, i32 0}
!3 = metadata !{i32 3, i32 3, i32 3, i32 0}
!4 = metadata !{metadata !"char16*", metadata !"char*", metadata !"char*", metadata !"image2d_t"}
!5 = metadata !{metadata !"pBuff0", metadata !"pBuff1", metadata !"pBuff2", metadata !"test_image"}
