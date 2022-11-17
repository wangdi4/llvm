; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S < %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(<4 x i8> %c, <4 x i8> %uc, <4 x i16> %s, <4 x i16> %us, <4 x i32> %i, <4 x i32> %ui, <4 x float> %f, <4 x float> addrspace(1)* nocapture %result) {
  ret void
}

; CHECK: define void @A(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle)
; CHECK: %0 = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK: %1 = bitcast i8* %0 to <4 x i8>*
; CHECK: %explicit_0 = load <4 x i8>, <4 x i8>* %1, align 4
; CHECK: %2 = getelementptr i8, i8* %UniformArgs, i32 4
; CHECK: %3 = bitcast i8* %2 to <4 x i8>*
; CHECK: %explicit_1 = load <4 x i8>, <4 x i8>* %3, align 4
; CHECK: %4 = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK: %5 = bitcast i8* %4 to <4 x i16>*
; CHECK: %explicit_2 = load <4 x i16>, <4 x i16>* %5, align 8
; CHECK: %6 = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK: %7 = bitcast i8* %6 to <4 x i16>*
; CHECK: %explicit_3 = load <4 x i16>, <4 x i16>* %7, align 8
; CHECK: %8 = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK: %9 = bitcast i8* %8 to <4 x i32>*
; CHECK: %explicit_4 = load <4 x i32>, <4 x i32>* %9, align 16
; CHECK: %10 = getelementptr i8, i8* %UniformArgs, i32 48
; CHECK: %11 = bitcast i8* %10 to <4 x i32>*
; CHECK: %explicit_5 = load <4 x i32>, <4 x i32>* %11, align 16
; CHECK: %12 = getelementptr i8, i8* %UniformArgs, i32 64
; CHECK: %13 = bitcast i8* %12 to <4 x float>*
; CHECK: %explicit_6 = load <4 x float>, <4 x float>* %13, align 16
; CHECK: %14 = getelementptr i8, i8* %UniformArgs, i32 80
; CHECK: %15 = bitcast i8* %14 to <4 x float> addrspace(1)**
; CHECK: %explicit_7 = load <4 x float> addrspace(1)*, <4 x float> addrspace(1)** %15, align 8
; CHECK: %16 = getelementptr i8, i8* %UniformArgs, i32 88
; CHECK: %pWorkDim = bitcast i8* %16 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; CHECK: %17 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i64, i64* %17
; CHECK: %18 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i64, i64* %18
; CHECK: %19 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i64, i64* %19
; CHECK: %20 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64, i64* %20
; CHECK: %21 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64, i64* %21
; CHECK: %22 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64, i64* %22
; CHECK: %23 = getelementptr i64, i64* %pWGId, i32 0
; CHECK: %GroupID_0 = load i64, i64* %23
; CHECK: %24 = getelementptr i64, i64* %pWGId, i32 1
; CHECK: %GroupID_1 = load i64, i64* %24
; CHECK: %25 = getelementptr i64, i64* %pWGId, i32 2
; CHECK: %GroupID_2 = load i64, i64* %25
; CHECK: %26 = mul i64 %InternalLocalSize_0, %GroupID_0
; CHECK: %27 = add i64 %26, %GlobalOffset_0
; CHECK: %28 = mul i64 %InternalLocalSize_1, %GroupID_1
; CHECK: %29 = add i64 %28, %GlobalOffset_1
; CHECK: %30 = mul i64 %InternalLocalSize_2, %GroupID_2
; CHECK: %31 = add i64 %30, %GlobalOffset_2
; CHECK: %32 = insertvalue [4 x i64] undef, i64 %27, 0
; CHECK: %33 = insertvalue [4 x i64] %32, i64 %29, 1
; CHECK: %BaseGlbId = insertvalue [4 x i64] %33, i64 %31, 2
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %InternalLocalSize_0, %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 0, %LocalSizeProd
; CHECK-NEXT: %pSpecialBuf = alloca i8, i64 %BarrierBufferSize, align 128
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{void (<4 x i8>, <4 x i8>, <4 x i16>, <4 x i16>, <4 x i32>, <4 x i32>, <4 x float>, <4 x float> addrspace(1)*)* @A}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-57: WARNING: Instruction with empty DebugLoc in function A {{.*}}
; DEBUGIFY-NOT: WARNING
