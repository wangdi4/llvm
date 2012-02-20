; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; XFAIL: win32
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct.PaddedDimId = type <{ [4 x i64] }>
%struct.WorkDim = type { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }

declare i32 @___Z9test_funccf_original(i8 signext, float) nounwind readnone

declare i32 @___Z9test_funcDv4_if_original(<4 x i32>, float) nounwind readnone

declare i32 @___Z9test_funcDv2_ld_original(<2 x i64>, double) nounwind readnone

declare void @__overloadingOrderTest_original(i32 addrspace(1)* nocapture) nounwind

declare void @dummybarrier.()

declare void @barrier(i64)

declare i8* @get_special_buffer.()

declare i64 @get_iter_count.()

define i32 @_Z9test_funccf(i8 signext %a, float %b, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone {
entry:
  ret i32 1
}

define i32 @_Z9test_funcDv4_if(<4 x i32> %a, float %b, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone {
entry:
  ret i32 2
}

define i32 @_Z9test_funcDv2_ld(<2 x i64> %a, double %b, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind readnone {
entry:
  ret i32 3
}

define void @__overloadingOrderTest_separated_args(i32 addrspace(1)* nocapture %result, i8 addrspace(3)* %pLocalMem, %struct.WorkDim* %pWorkDim, i64* %pWGId, %struct.PaddedDimId* %pBaseGlbId, %struct.PaddedDimId* %pLocalIds, i64* %contextpointer, i64 %iterCount, i8* %pSpecialBuf, i64* %pCurrWI) nounwind alwaysinline {
entry:
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %thenBB, %entry
  %CurrWI..0 = phi i64 [ %"CurrWI++", %thenBB ], [ 0, %entry ]
  %check.WI.iter = icmp ult i64 %CurrWI..0, %iterCount
  br i1 %check.WI.iter, label %thenBB, label %elseBB

thenBB:                                           ; preds = %"Barrier BB"
  %"CurrWI++" = add nuw i64 %CurrWI..0, 1
  br label %"Barrier BB"

elseBB:                                           ; preds = %"Barrier BB"
  store i32 2, i32 addrspace(1)* %result, align 4
  ret void
}

define void @overloadingOrderTest(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to i32 addrspace(1)**
  %1 = load i32 addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 56
  %3 = bitcast i8* %2 to i64*
  %4 = load i64* %3, align 8
  br label %"Barrier BB.i"

"Barrier BB.i":                                   ; preds = %thenBB.i, %entry
  %CurrWI..0.i = phi i64 [ %"CurrWI++.i", %thenBB.i ], [ 0, %entry ]
  %check.WI.iter.i = icmp ult i64 %CurrWI..0.i, %4
  br i1 %check.WI.iter.i, label %thenBB.i, label %__overloadingOrderTest_separated_args.exit

thenBB.i:                                         ; preds = %"Barrier BB.i"
  %"CurrWI++.i" = add nuw i64 %CurrWI..0.i, 1
  br label %"Barrier BB.i"

__overloadingOrderTest_separated_args.exit:       ; preds = %"Barrier BB.i"
  store i32 2, i32 addrspace(1)* %1, align 4
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, %struct.WorkDim*, i64*, %struct.PaddedDimId*, %struct.PaddedDimId*, i64*, i64, i8*, i64*)* @__overloadingOrderTest_separated_args, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *", metadata !"opencl_overloadingOrderTest_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !"", void (i8*)* @overloadingOrderTest}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int*"}
!5 = metadata !{metadata !"result"}
