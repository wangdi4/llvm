; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare void @__k_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture) nounwind

define float @_Z3cosf(float %x) nounwind readnone {
entry:
  %call = tail call x86_svmlcc float @__ocl_svml_b1_cosf1(float %x) nounwind readnone
  ret float %call
}

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define void @__k_separated_args(float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  br label %SyncBB

SyncBB:                                           ; preds = %entry, %thenBB
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %entry ]
  %tmp1 = load float addrspace(1)* %b, align 4
  %call.i = tail call x86_svmlcc float @__ocl_svml_b1_cosf1(float %tmp1) nounwind readnone
  store float %call.i, float addrspace(1)* %a, align 4
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %SyncBB1

thenBB:                                           ; preds = %SyncBB
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %SyncBB

SyncBB1:                                          ; preds = %SyncBB
  ret void
}

declare float @__ocl_svml_b1_cosf1(float) readnone

define void @k(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 64
  %6 = bitcast i8* %5 to i64*
  %7 = load i64* %6, align 8
  br label %SyncBB.i

SyncBB.i:                                         ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %tmp1.i = load float addrspace(1)* %4, align 4
  %call.i.i = call x86_svmlcc float @__ocl_svml_b1_cosf1(float %tmp1.i) nounwind readnone
  store float %call.i.i, float addrspace(1)* %1, align 4
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %7
  br i1 %check.WI.iter.i, label %thenBB.i, label %__k_separated_args.exit

thenBB.i:                                         ; preds = %SyncBB.i
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %SyncBB.i

__k_separated_args.exit:                          ; preds = %SyncBB.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__k_separated_args, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *", metadata !"opencl_k_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @k}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 1}
!3 = metadata !{i32 3, i32 3}
!4 = metadata !{metadata !"float*", metadata !"float*"}
!5 = metadata !{metadata !"a", metadata !"b"}
