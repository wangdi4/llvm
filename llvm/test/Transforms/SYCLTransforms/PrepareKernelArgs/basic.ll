; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %a, i32 %b) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  ret void
}

; CHECK: define void @A(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle)
; CHECK:     %0 = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK:     %explicit_0 = load ptr addrspace(1), ptr %0, align 8
; CHECK:     %1 = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK:     %explicit_1 = load ptr addrspace(1), ptr %1, align 8
; CHECK:     %2 = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK:     %explicit_2 = load i32, ptr %2, align 4
; CHECK-ARG: %pWorkDim = getelementptr i8, ptr %UniformArgs, i32 24
; CHECK-TLS: %3 = getelementptr i8, ptr %UniformArgs, i32 24

; CHECK-TLS: store ptr [[BC:%[a-zA-Z0-9]+]], ptr @__pWorkDim

; CHECK-TLS: store ptr %pWGId, ptr @__pWGId

; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 8, i32 0, i32 0
; CHECK:     %InternalLocalSize_0 = load i64, ptr [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 8, i32 0, i32 1
; CHECK:     %InternalLocalSize_1 = load i64, ptr [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 8, i32 0, i32 2
; CHECK:     %InternalLocalSize_2 = load i64, ptr [[GEP]]

; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 0
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 1, i32 0
; CHECK:     %GlobalOffset_0 = load i64, ptr [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 1
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 1, i32 1
; CHECK:     %GlobalOffset_1 = load i64, ptr [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 2
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[BC]], i32 0, i32 1, i32 2
; CHECK:     %GlobalOffset_2 = load i64, ptr [[GEP]]

; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 0
; CHECK:     %GroupID_0 = load i64, ptr [[GEP]] 
; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 1
; CHECK:     %GroupID_1 = load i64, ptr [[GEP]]
; CHECK:     [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 2
; CHECK:     %GroupID_2 = load i64, ptr [[GEP]]
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
; CHECK-TLS: store [4 x i64] [[IV2]], ptr @__BaseGlbId
; CHECK:     ret void

!sycl.kernels = !{!0}
!0 = !{ptr @A}
!1 = !{!"int*", !"int*", !"int"}
!2 = !{ptr addrspace(1) null, ptr addrspace(1) null, i32 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-34: WARNING: Instruction with empty DebugLoc in function A
; DEBUGIFY-NOT: WARNING

; DEBUGIFY-TLS-NOT: WARNING
; DEBUGIFY-TLS-COUNT-38: WARNING: Instruction with empty DebugLoc in function A
; DEBUGIFY-TLS-NOT: WARNING
