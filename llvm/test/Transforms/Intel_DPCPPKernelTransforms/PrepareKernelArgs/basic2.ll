; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S < %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(i32 addrspace(1)* nocapture %out, i32 %b, i32 addrspace(1)* nocapture %a) {
  ret void
}

; CHECK: define void @A(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle)
; CHECK: %0 = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK: %1 = bitcast i8* %0 to i32 addrspace(1)**
; CHECK: %explicit_0 = load i32 addrspace(1)*, i32 addrspace(1)** %1, align 8
; CHECK: %2 = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK: %3 = bitcast i8* %2 to i32*
; CHECK: %explicit_1 = load i32, i32* %3, align 4
; CHECK: %4 = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK: %5 = bitcast i8* %4 to i32 addrspace(1)**
; CHECK: %explicit_2 = load i32 addrspace(1)*, i32 addrspace(1)** %5, align 8
; CHECK: %6 = getelementptr i8, i8* %UniformArgs, i32 24
; CHECK: %pWorkDim = bitcast i8* %6 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; CHECK: %7 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i64, i64* %7
; CHECK: %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i64, i64* %8
; CHECK: %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i64, i64* %9
; CHECK: %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64, i64* %10
; CHECK: %11 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64, i64* %11
; CHECK: %12 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64, i64* %12
; CHECK: %13 = getelementptr i64, i64* %pWGId, i32 0
; CHECK: %GroupID_0 = load i64, i64* %13
; CHECK: %14 = getelementptr i64, i64* %pWGId, i32 1
; CHECK: %GroupID_1 = load i64, i64* %14
; CHECK: %15 = getelementptr i64, i64* %pWGId, i32 2
; CHECK: %GroupID_2 = load i64, i64* %15
; CHECK: %16 = mul i64 %InternalLocalSize_0, %GroupID_0
; CHECK: %17 = add i64 %16, %GlobalOffset_0
; CHECK: %18 = mul i64 %InternalLocalSize_1, %GroupID_1
; CHECK: %19 = add i64 %18, %GlobalOffset_1
; CHECK: %20 = mul i64 %InternalLocalSize_2, %GroupID_2
; CHECK: %21 = add i64 %20, %GlobalOffset_2
; CHECK: %22 = insertvalue [4 x i64] undef, i64 %17, 0
; CHECK: %23 = insertvalue [4 x i64] %22, i64 %19, 1
; CHECK: %BaseGlbId = insertvalue [4 x i64] %23, i64 %21, 2
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %InternalLocalSize_0, %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 0, %LocalSizeProd
; CHECK: %pSpecialBuf = alloca i8, i64 %BarrierBufferSize, align 128
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*, i32, i32 addrspace(1)*)* @A}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function A {{.*}}
; DEBUGIFY-NOT: WARNING
