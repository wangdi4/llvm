; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__k_original(<16 x float> addrspace(1)* nocapture) nounwind

declare i64 @get_global_id(i32) readnone

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

declare i64 @get_new_global_id.(i32, i64)

define void @__k_separated_args(<16 x float> addrspace(1)* nocapture %a, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  br label %SyncBB

SyncBB:                                           ; preds = %entry, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %entry ]
  %0 = getelementptr %struct.PaddedDimId* %pLocalIds, i64 %CurrWI..0, i32 0, i64 0
  %1 = load i64* %0, align 8
  %2 = getelementptr %struct.PaddedDimId* %pBaseGlbId, i64 0, i32 0, i64 0
  %3 = load i64* %2, align 8
  %4 = add i64 %1, %3
  %arrayidx = getelementptr inbounds <16 x float> addrspace(1)* %a, i64 %4
  store <16 x float> zeroinitializer, <16 x float> addrspace(1)* %arrayidx, align 64
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %SyncBB
  ret void
}

define void @k(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to <16 x float> addrspace(1)**
  %1 = load <16 x float> addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 32
  %3 = bitcast i8* %2 to %struct.PaddedDimId**
  %4 = load %struct.PaddedDimId** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 40
  %6 = bitcast i8* %5 to %struct.PaddedDimId**
  %7 = load %struct.PaddedDimId** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 56
  %9 = bitcast i8* %8 to i64*
  %10 = load i64* %9, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %11 = getelementptr %struct.PaddedDimId* %7, i64 %CurrWI..0.i, i32 0, i64 0
  %12 = load i64* %11, align 8
  %13 = getelementptr %struct.PaddedDimId* %4, i64 0, i32 0, i64 0
  %14 = load i64* %13, align 8
  %15 = add i64 %12, %14
  %arrayidx.i = getelementptr inbounds <16 x float> addrspace(1)* %1, i64 %15
  store <16 x float> zeroinitializer, <16 x float> addrspace(1)* %arrayidx.i, align 64
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %10
  br i1 %check.WI.iter.i, label %thenBB.i, label %__k_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__k_separated_args.exit:                          ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (<16 x float> addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__k_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float16 __attribute__((address_space(1))) *", metadata !"opencl_k_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @k}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"float16*"}
!5 = metadata !{metadata !"a"}
