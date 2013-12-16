; RUN: oclopt -prepare-kernel-args -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(<4 x i8> %c, <4 x i8> %uc, <4 x i16> %s, <4 x i16> %us, <4 x i32> %i, <4 x i32> %ui, <4 x float> %f, <4 x float> addrspace(1)* nocapture %result, i8 addrspace(3)* %pLocalMemBase, { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, i64* %pCurrWI, {}* %RuntimeHandle) nounwind {

; CHECK: define void @A(i8* noalias %pUniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle) {
; CHECK: %0 = getelementptr i8* %pUniformArgs, i32 0
; CHECK: %1 = bitcast i8* %0 to <4 x i8>*
; CHECK: %explicit_0 = load <4 x i8>* %1, align 4
; CHECK: %2 = getelementptr i8* %pUniformArgs, i32 4
; CHECK: %3 = bitcast i8* %2 to <4 x i8>*
; CHECK: %explicit_1 = load <4 x i8>* %3, align 4
; CHECK: %4 = getelementptr i8* %pUniformArgs, i32 8
; CHECK: %5 = bitcast i8* %4 to <4 x i16>*
; CHECK: %explicit_2 = load <4 x i16>* %5, align 8
; CHECK: %6 = getelementptr i8* %pUniformArgs, i32 16
; CHECK: %7 = bitcast i8* %6 to <4 x i16>*
; CHECK: %explicit_3 = load <4 x i16>* %7, align 8
; CHECK: %8 = getelementptr i8* %pUniformArgs, i32 32
; CHECK: %9 = bitcast i8* %8 to <4 x i32>*
; CHECK: %explicit_4 = load <4 x i32>* %9, align 16
; CHECK: %10 = getelementptr i8* %pUniformArgs, i32 48
; CHECK: %11 = bitcast i8* %10 to <4 x i32>*
; CHECK: %explicit_5 = load <4 x i32>* %11, align 16
; CHECK: %12 = getelementptr i8* %pUniformArgs, i32 64
; CHECK: %13 = bitcast i8* %12 to <4 x float>*
; CHECK: %explicit_6 = load <4 x float>* %13, align 16
; CHECK: %14 = getelementptr i8* %pUniformArgs, i32 80
; CHECK: %15 = bitcast i8* %14 to <4 x float> addrspace(1)**
; CHECK: %explicit_7 = load <4 x float> addrspace(1)** %15, align 8
; CHECK: %16 = alloca [0 x i8], align 128
; CHECK: %pLocalMemBase = bitcast [0 x i8]* %16 to i8 addrspace(3)*
; CHECK: %17 = getelementptr i8* %pUniformArgs, i32 88
; CHECK: %pWorkDim = bitcast i8* %17 to { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }*
; CHECK: %18 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 3, i32 0
; CHECK: %LocalSize_0 = load i64* %18
; CHECK: %19 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 3, i32 1
; CHECK: %LocalSize_1 = load i64* %19
; CHECK: %20 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 3, i32 2
; CHECK: %LocalSize_2 = load i64* %20
; CHECK: %21 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64* %21
; CHECK: %22 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64* %22
; CHECK: %23 = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64* %23
; CHECK: %24 = getelementptr i64* %pWGId, i32 0
; CHECK: %GroupID_0 = load i64* %24
; CHECK: %25 = getelementptr i64* %pWGId, i32 1
; CHECK: %GroupID_1 = load i64* %25
; CHECK: %26 = getelementptr i64* %pWGId, i32 2
; CHECK: %GroupID_2 = load i64* %26
; CHECK: %27 = mul i64 %LocalSize_0, %GroupID_0
; CHECK: %28 = add i64 %27, %GlobalOffset_0
; CHECK: %29 = mul i64 %LocalSize_1, %GroupID_1
; CHECK: %30 = add i64 %29, %GlobalOffset_1
; CHECK: %31 = mul i64 %LocalSize_2, %GroupID_2
; CHECK: %32 = add i64 %31, %GlobalOffset_2
; CHECK: %33 = insertvalue [4 x i64] undef, i64 %28, 0
; CHECK: %34 = insertvalue [4 x i64] %33, i64 %30, 1
; CHECK: %BaseGlbId = insertvalue [4 x i64] %34, i64 %32, 2
; CHECK: %35 = mul i64 0, %LocalSize_0
; CHECK: %36 = mul i64 %35, %LocalSize_1
; CHECK: %BarrierBufferSize = mul i64 %36, %LocalSize_2
; CHECK: %pSpecialBuf = alloca i8, i64 %BarrierBufferSize, align 128
; CHECK: %pCurrWI = alloca i64
; CHECK: call void @__A_separated_args(<4 x i8> %explicit_0, <4 x i8> %explicit_1, <4 x i16> %explicit_2, <4 x i16> %explicit_3, <4 x i32> %explicit_4, <4 x i32> %explicit_5, <4 x float> %explicit_6, <4 x float> addrspace(1)* %explicit_7, i8 addrspace(3)* %pLocalMemBase, { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }* %pWorkDim, i64* %pWGId, [4 x i64] %BaseGlbId, i8* %pSpecialBuf, i64* %pCurrWI, {}* %RuntimeHandle)
; CHECK: ret void

  ret void
}


!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!opencl.kernel_info = !{!9}
!llvm.functions_info = !{}

!0 = metadata !{void (<4 x i8>, <4 x i8>, <4 x i16>, <4 x i16>, <4 x i32>, <4 x i32>, <4 x float>, <4 x float> addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }*, i64*, [4 x i64], i8*, i64*, {}*)* @A, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"char4", metadata !"uchar4", metadata !"short4", metadata !"ushort4", metadata !"int4", metadata !"uint4", metadata !"float4", metadata !"float4*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"c", metadata !"uc", metadata !"s", metadata !"us", metadata !"i", metadata !"ui", metadata !"f", metadata !"result"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
!9 = metadata !{void (<4 x i8>, <4 x i8>, <4 x i16>, <4 x i16>, <4 x i32>, <4 x i32>, <4 x float>, <4 x float> addrspace(1)*, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], i64, {}*, [4 x i64]* }*, i64*, [4 x i64], i8*, i64*, {}*)* @A, metadata !10}
!10 = metadata !{metadata !11, metadata !12, metadata !13, metadata !14, metadata !15, metadata !16, metadata !17, metadata !18, metadata !19}
!11 = metadata !{metadata !"local_buffer_size", i32 0}
!12 = metadata !{metadata !"barrier_buffer_size", i32 0}
!13 = metadata !{metadata !"kernel_execution_length", i32 20}
!14 = metadata !{metadata !"kernel_has_barrier", i1 false}
!15 = metadata !{metadata !"no_barrier_path", i1 true}
!16 = metadata !{metadata !"vectorized_kernel", null}
!17 = metadata !{metadata !"vectorized_width", null}
!18 = metadata !{metadata !"kernel_wrapper", null}
!19 = metadata !{metadata !"scalarized_kernel", null}
