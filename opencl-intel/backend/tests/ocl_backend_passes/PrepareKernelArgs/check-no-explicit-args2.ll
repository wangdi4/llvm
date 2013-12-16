; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1(i8 %arg1) nounwind {
entry:
  ret void
}

;;explicit args
;;char arg1 - expected alignment: 1
; CHECK: [[ARG0_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = load i8* [[ARG0_BUFF_IDX]], align 1
;;implicit args
; CHECK-NEXT: [[IMPLCT_ARG0_SLMBUFF:%[a-zA-Z0-9]+]] = alloca [0 x i8], align 128
; CHECK-NEXT: %pLocalMemBase = bitcast [0 x i8]* [[IMPLCT_ARG0_SLMBUFF]] to i8 addrspace(3)*
; CHECK-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr i8* %pUniformArgs, i32 4
; CHECK-NEXT: %pWorkDim = bitcast i8* [[GEP0]] to { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }*
; CHECK-NEXT: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 3, i32 0
; CHECK-NEXT: %LocalSize_0 = load i32* [[GEP1]]
; CHECK-NEXT: [[GEP2:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 3, i32 1
; CHECK-NEXT: %LocalSize_1 = load i32* [[GEP2]]
; CHECK-NEXT: [[GEP3:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 3, i32 2
; CHECK-NEXT: %LocalSize_2 = load i32* [[GEP3]]
; CHECK-NEXT: [[GEP4:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK-NEXT: %GlobalOffset_0 = load i32* [[GEP4]]
; CHECK-NEXT: [[GEP5:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK-NEXT: %GlobalOffset_1 = load i32* [[GEP5]]
; CHECK-NEXT: [[GEP6:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK-NEXT: %GlobalOffset_2 = load i32* [[GEP6]]
; CHECK-NEXT: [[GEP7:%[a-zA-Z0-9]+]] = getelementptr i32* %pWGId, i32 0
; CHECK-NEXT: %GroupID_0 = load i32* [[GEP7]]
; CHECK-NEXT: [[GEP8:%[a-zA-Z0-9]+]] = getelementptr i32* %pWGId, i32 1
; CHECK-NEXT: %GroupID_1 = load i32* [[GEP8]]
; CHECK-NEXT: [[GEP9:%[a-zA-Z0-9]+]] = getelementptr i32* %pWGId, i32 2
; CHECK-NEXT: %GroupID_2 = load i32* [[GEP9]]
; CHECK-NEXT: [[MUL0:%[a-zA-Z0-9]+]] = mul i32 %LocalSize_0, %GroupID_0
; CHECK-NEXT: [[ADD0:%[a-zA-Z0-9]+]] = add i32 [[MUL0]], %GlobalOffset_0
; CHECK-NEXT: [[MUL1:%[a-zA-Z0-9]+]] = mul i32 %LocalSize_1, %GroupID_1
; CHECK-NEXT: [[ADD1:%[a-zA-Z0-9]+]] = add i32 [[MUL1]], %GlobalOffset_1
; CHECK-NEXT: [[MUL2:%[a-zA-Z0-9]+]] = mul i32 %LocalSize_2, %GroupID_2
; CHECK-NEXT: [[ADD2:%[a-zA-Z0-9]+]] = add i32 [[MUL2]], %GlobalOffset_2
; CHECK-NEXT: [[IV0:%[a-zA-Z0-9]+]] = insertvalue [4 x i32] undef, i32 [[ADD0]], 0
; CHECK-NEXT: [[IV1:%[a-zA-Z0-9]+]] = insertvalue [4 x i32] [[IV0]], i32 [[ADD1]], 1
; CHECK-NEXT: %BaseGlbId = insertvalue [4 x i32] [[IV1]], i32 [[ADD2]], 2
; CHECK-NEXT: [[BBS0:%[a-zA-Z0-9]+]] = mul i32 0, %LocalSize_0
; CHECK-NEXT: [[BBS1:%[a-zA-Z0-9]+]] = mul i32 [[BBS0]], %LocalSize_1
; CHECK-NEXT: %BarrierBufferSize = mul i32 [[BBS1]], %LocalSize_2
; CHECK-NEXT: %pSpecialBuf = alloca i8, i32 %BarrierBufferSize, align 128
; CHECK-NEXT: %pCurrWI = alloca i32
; CHECK-NEXT: call void @__t1_separated_args(i8 %explicit_0, i8 addrspace(3)* %pLocalMemBase, { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32], i32, {}*, [4 x i32]* }* %pWorkDim, i32* %pWGId, [4 x i32] %BaseGlbId, i8* %pSpecialBuf, i32* %pCurrWI, {}* %RuntimeHandle)
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (i8)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"char"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"const"}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
