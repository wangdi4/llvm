; RUN: oclopt -add-implicit-args -prepare-kernel-args -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %a, i32 %b) nounwind {
  ret void
}

; CHECK: define void @A(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
; CHECK:     %0 = getelementptr i8* %pUniformArgs, i32 0
; CHECK:     %1 = bitcast i8* %0 to i32 addrspace(1)**
; CHECK:     %explicit_0 = load i32 addrspace(1)** %1, align 8
; CHECK:     %2 = getelementptr i8* %pUniformArgs, i32 8
; CHECK:     %3 = bitcast i8* %2 to i32 addrspace(1)**
; CHECK:     %explicit_1 = load i32 addrspace(1)** %3, align 8
; CHECK:     %4 = getelementptr i8* %pUniformArgs, i32 16
; CHECK:     %5 = bitcast i8* %4 to i32*
; CHECK:     %explicit_2 = load i32* %5, align 4
; CHECK:     %6 = getelementptr i8* %pUniformArgs, i32 24
; CHECK:     %pWorkDim = bitcast i8* %6 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*
; CHECK:     %7 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 0
; CHECK:     %LocalSize_0 = load i64* %7
; CHECK:     %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 1
; CHECK:     %LocalSize_1 = load i64* %8
; CHECK:     %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 2
; CHECK:     %LocalSize_2 = load i64* %9
; CHECK:     %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK:     %GlobalOffset_0 = load i64* %10
; CHECK:     %11 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK:     %GlobalOffset_1 = load i64* %11
; CHECK:     %12 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK:     %GlobalOffset_2 = load i64* %12
; CHECK:     %13 = getelementptr i64* %pWGId, i32 0
; CHECK:     %GroupID_0 = load i64* %13
; CHECK:     %14 = getelementptr i64* %pWGId, i32 1
; CHECK:     %GroupID_1 = load i64* %14
; CHECK:     %15 = getelementptr i64* %pWGId, i32 2
; CHECK:     %GroupID_2 = load i64* %15
; CHECK:     %16 = mul i64 %LocalSize_0, %GroupID_0
; CHECK:     %17 = add i64 %16, %GlobalOffset_0
; CHECK:     %18 = mul i64 %LocalSize_1, %GroupID_1
; CHECK:     %19 = add i64 %18, %GlobalOffset_1
; CHECK:     %20 = mul i64 %LocalSize_2, %GroupID_2
; CHECK:     %21 = add i64 %20, %GlobalOffset_2
; CHECK:     %22 = insertvalue [4 x i64] undef, i64 %17, 0
; CHECK:     %23 = insertvalue [4 x i64] %22, i64 %19, 1
; CHECK:     %BaseGlbId = insertvalue [4 x i64] %23, i64 %21, 2
; CHECK:     %24 = mul i64 0, %LocalSize_0
; CHECK:     %25 = mul i64 %24, %LocalSize_1
; CHECK:     %BarrierBufferSize = mul i64 %25, %LocalSize_2
; CHECK:     %pSpecialBuf = alloca i8, i64 %BarrierBufferSize, align 128
; CHECK:     call void @__A_separated_args(i32 addrspace(1)* %explicit_0, i32 addrspace(1)* %explicit_1, i32 %explicit_2, i8 addrspace(3)* null, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, {}* %RuntimeHandle)
; CHECK:     ret void
; CHECK: attributes #1 = { alwaysinline nounwind }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!10}
!opencl.ocl.version = !{!11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}
!opencl.kernel_info = !{!13}
!opencl.module_info_list = !{}
!llvm.functions_info = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @A, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"uint*", !"uint*", !"uint"}
!4 = !{!"kernel_arg_type_qual", !"", !"const", !""}
!5 = !{!"kernel_arg_name", !"out", !"a", !"b"}
!10 = !{i32 1, i32 0}
!11 = !{i32 0, i32 0}
!12 = !{}
!13 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @A, !14}
!14 = !{!15, !16, !17, !18, !19, !20, !21, !22, !23}
!15 = !{!"local_buffer_size", i32 0}
!16 = !{!"barrier_buffer_size", i32 0}
!17 = !{!"kernel_execution_length", i32 12}
!18 = !{!"kernel_has_barrier", i1 false}
!19 = !{!"no_barrier_path", i1 true}
!20 = !{!"vectorized_kernel", null}
!21 = !{!"vectorized_width", null}
!22 = !{!"kernel_wrapper", null}
!23 = !{!"scalarized_kernel", null}
