; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-add-tls-globals -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-add-tls-globals -dpcpp-kernel-prepare-args -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-add-tls-globals,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-add-tls-globals,dpcpp-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(i32 addrspace(1)* nocapture %out, i32 addrspace(1)* nocapture %a, i32 %b) {
  ret void
}

; CHECK: define void @A(i8* noalias %UniformArgs, i64* noalias %pWGId, {}* noalias %RuntimeHandle)
; CHECK:     %0 = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK:     %1 = bitcast i8* %0 to i32 addrspace(1)**
; CHECK:     %explicit_0 = load i32 addrspace(1)*, i32 addrspace(1)** %1, align 8
; CHECK:     %2 = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK:     %3 = bitcast i8* %2 to i32 addrspace(1)**
; CHECK:     %explicit_1 = load i32 addrspace(1)*, i32 addrspace(1)** %3, align 8
; CHECK:     %4 = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK:     %5 = bitcast i8* %4 to i32*
; CHECK:     %explicit_2 = load i32, i32* %5, align 4
; CHECK:     %6 = getelementptr i8, i8* %UniformArgs, i32 24

; CHECK-ARG: %pWorkDim = bitcast i8* %6 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; CHECK-TLS: [[BC:%[a-zA-Z0-9]+]] = bitcast i8* %6 to { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*
; CHECK-TLS: store { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }** @pWorkDim

; CHECK-TLS: store i64* %pWGId, i64** @pWGId

; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 8, i32 0, i32 0
; CHECK:     %InternalLocalSize_0 = load i64, i64* [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 8, i32 0, i32 1
; CHECK:     %InternalLocalSize_1 = load i64, i64* [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 8, i32 0, i32 2
; CHECK:     %InternalLocalSize_2 = load i64, i64* [[GEP]]

; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 1, i32 0
; CHECK:     %GlobalOffset_0 = load i64, i64* [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 1, i32 1
; CHECK:     %GlobalOffset_1 = load i64, i64* [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* [[BC]], i32 0, i32 1, i32 2
; CHECK:     %GlobalOffset_2 = load i64, i64* [[GEP]]

; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, i64* %pWGId, i32 0
; CHECK:     %GroupID_0 = load i64, i64* [[GEP]]
; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, i64* %pWGId, i32 1
; CHECK:     %GroupID_1 = load i64, i64* [[GEP]]
; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, i64* %pWGId, i32 2
; CHECK:     %GroupID_2 = load i64, i64* [[GEP]]
; CHECK:     [[MUL0:%[a-zA-Z0-9]+]] = mul i64 %InternalLocalSize_0, %GroupID_0
; CHECK:     [[ADD0:%[a-zA-Z0-9]+]] = add i64 [[MUL0]], %GlobalOffset_0
; CHECK:     [[MUL1:%[a-zA-Z0-9]+]] = mul i64 %InternalLocalSize_1, %GroupID_1
; CHECK:     [[ADD1:%[a-zA-Z0-9]+]] = add i64 [[MUL1]], %GlobalOffset_1
; CHECK:     [[MUL2:%[a-zA-Z0-9]+]] = mul i64 %InternalLocalSize_2, %GroupID_2
; CHECK:     [[ADD2:%[a-zA-Z0-9]+]] = add i64 [[MUL2]], %GlobalOffset_2
; CHECK:     [[IV0:%[a-zA-Z0-9]+]] = insertvalue [4 x i64] undef, i64 [[ADD0]], 0
; CHECK:     [[IV1:%[a-zA-Z0-9]+]] = insertvalue [4 x i64] [[IV0]], i64 [[ADD1]], 1
; CHECK-ARG: %BaseGlbId = insertvalue [4 x i64] [[IV1]], i64 [[ADD2]], 2
; CHECK-TLS: [[IV2:%[a-zA-Z0-9]+]] = insertvalue [4 x i64] [[IV1]], i64 [[ADD2]], 2
; CHECK-TLS: store [4 x i64] [[IV2]], [4 x i64]* @BaseGlbId
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %InternalLocalSize_0, %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 0, %LocalSizeProd
; CHECK-ARG: %pSpecialBuf = alloca i8, i64 %BarrierBufferSize, align 128
; CHECK:     ret void

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32)* @A}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function A
; DEBUGIFY-NOT: WARNING

; DEBUGIFY-TLS-NOT: WARNING
; DEBUGIFY-TLS-COUNT-48: WARNING: Instruction with empty DebugLoc in function A
; DEBUGIFY-TLS-NOT: WARNING
