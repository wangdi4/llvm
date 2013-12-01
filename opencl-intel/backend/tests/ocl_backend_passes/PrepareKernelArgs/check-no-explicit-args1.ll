; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1() nounwind {
entry:
  ret void
}

;;implicit args
; CHECK: [[IMPLCT_ARG0_SLMBUFF:%[a-zA-Z0-9]+]] = alloca [0 x i8], align 128
; CHECK-NEXT: [[IMPLCT_ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast [0 x i8]* [[IMPLCT_ARG0_SLMBUFF]] to i8 addrspace(3)*

; CHECK-NEXT: [[IMPLCT_ARG1_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK-NEXT: [[IMPLCT_ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG1_BUFF_IDX]] to { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }**
; CHECK-NEXT: [[IMPLCT_ARG1:%[a-zA-Z0-9]+]] = load { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }** [[IMPLCT_ARG1_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG2_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 4
; CHECK-NEXT: [[IMPLCT_ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG2_BUFF_IDX]] to i32**
; CHECK-NEXT: [[IMPLCT_ARG2:%[a-zA-Z0-9]+]] = load i32** [[IMPLCT_ARG2_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG3_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 8
; CHECK-NEXT: [[IMPLCT_ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG3_BUFF_IDX]] to <{ [4 x i32] }>**
; CHECK-NEXT: [[IMPLCT_ARG3:%[a-zA-Z0-9]+]] = load <{ [4 x i32] }>** [[IMPLCT_ARG3_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG4_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 12
; CHECK-NEXT: [[IMPLCT_ARG4_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG4_BUFF_IDX]] to i32**
; CHECK-NEXT: [[IMPLCT_ARG4:%[a-zA-Z0-9]+]] = load i32** [[IMPLCT_ARG4_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG5_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 16
; CHECK-NEXT: [[IMPLCT_ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG5_BUFF_IDX]] to <{ [4 x i32] }>**
; CHECK-NEXT: [[IMPLCT_ARG5:%[a-zA-Z0-9]+]] = load <{ [4 x i32] }>** [[IMPLCT_ARG5_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG6_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 20
; CHECK-NEXT: [[IMPLCT_ARG6_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG6_BUFF_IDX]] to i32*
; CHECK-NEXT: [[IMPLCT_ARG6:%[a-zA-Z0-9]+]] = load i32* [[IMPLCT_ARG6_TYPECAST]], align 4

; CHECK-NEXT: [[IMPLCT_ARG7_DIM0_PLOCAL:%pLocalSize_dim0_]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* [[IMPLCT_ARG1]], i32 0, i32 3, i32 0
; CHECK-NEXT: [[IMPLCT_ARG7_DIM0_LOCAL:%LocalSize_dim0_]] = load i32* [[IMPLCT_ARG7_DIM0_PLOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_DIM0_BARRIER:%[a-zA-Z0-9]+]] = mul i32 0, [[IMPLCT_ARG7_DIM0_LOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_DIM1_PLOCAL:%pLocalSize_dim1_]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* [[IMPLCT_ARG1]], i32 0, i32 3, i32 1
; CHECK-NEXT: [[IMPLCT_ARG7_DIM1_LOCAL:%LocalSize_dim1_]] = load i32* [[IMPLCT_ARG7_DIM1_PLOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_DIM1_BARRIER:%[a-zA-Z0-9]+]] = mul i32 [[IMPLCT_ARG7_DIM0_BARRIER]], [[IMPLCT_ARG7_DIM1_LOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_DIM2_PLOCAL:%pLocalSize_dim2_]] = getelementptr { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* [[IMPLCT_ARG1]], i32 0, i32 3, i32 2
; CHECK-NEXT: [[IMPLCT_ARG7_DIM2_LOCAL:%LocalSize_dim2_]] = load i32* [[IMPLCT_ARG7_DIM2_PLOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_DIM2_BARRIER:%BarrierBufferSize]] = mul i32 [[IMPLCT_ARG7_DIM1_BARRIER]], [[IMPLCT_ARG7_DIM2_LOCAL]]
; CHECK-NEXT: [[IMPLCT_ARG7_BARRIER_BUFF:%BarrierBuffer]] = alloca i8, i32 [[IMPLCT_ARG7_DIM2_BARRIER]], align 128

; CHECK-NEXT: [[IMPLCT_ARG8_CURR_WORKITEM:%[a-zA-Z0-9]+]] = alloca i32

; CHECK-NEXT: [[IMPLCT_ARG9_BUFF_IDX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 24
; CHECK-NEXT: [[IMPLCT_ARG9_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[IMPLCT_ARG9_BUFF_IDX]] to %struct.ExtendedExecutionContext**
; CHECK-NEXT: [[IMPLCT_ARG9:%[a-zA-Z0-9]+]] = load %struct.ExtendedExecutionContext** [[IMPLCT_ARG9_TYPECAST]], align 4

;; call original func
; CHECK: call void @__t1_separated_args(i8 addrspace(3)* [[IMPLCT_ARG0_TYPECAST]], { i32, [3 x i32], [3 x i32], [3 x i32], [3 x i32] }* [[IMPLCT_ARG1]], i32* [[IMPLCT_ARG2]], <{ [4 x i32] }>* [[IMPLCT_ARG3]], i32* [[IMPLCT_ARG4]], <{ [4 x i32] }>* [[IMPLCT_ARG5]], i32 [[IMPLCT_ARG6]], i8* [[IMPLCT_ARG7_BARRIER_BUFF]], i32* [[IMPLCT_ARG8_CURR_WORKITEM]], %struct.ExtendedExecutionContext* [[IMPLCT_ARG9]])
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void ()* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space"}
!2 = metadata !{metadata !"kernel_arg_access_qual"}
!3 = metadata !{metadata !"kernel_arg_type"}
!4 = metadata !{metadata !"kernel_arg_type_qual"}
!5 = metadata !{metadata !"kernel_arg_name"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
