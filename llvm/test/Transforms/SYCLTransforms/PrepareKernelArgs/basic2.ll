; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(ptr addrspace(1) nocapture %out, i32 %b, ptr addrspace(1) nocapture %a) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  ret void
}

; CHECK: define void @A(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle)
; CHECK: %0 = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK: %explicit_0 = load ptr addrspace(1), ptr %0, align 8
; CHECK: %1 = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK: %explicit_1 = load i32, ptr %1, align 4
; CHECK: %2 = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK: %explicit_2 = load ptr addrspace(1), ptr %2, align 8
; CHECK: %pWorkDim = getelementptr i8, ptr %UniformArgs, i32 24
; CHECK: %3 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i64, ptr %3
; CHECK: %4 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i64, ptr %4
; CHECK: %5 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i64, ptr %5
; CHECK: %6 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64, ptr %6
; CHECK: %7 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64, ptr %7
; CHECK: %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64, ptr %8
; CHECK: %9 = getelementptr i64, ptr %pWGId, i32 0
; CHECK: %GroupID_0 = load i64, ptr %9
; CHECK: %10 = getelementptr i64, ptr %pWGId, i32 1
; CHECK: %GroupID_1 = load i64, ptr %10
; CHECK: %11 = getelementptr i64, ptr %pWGId, i32 2
; CHECK: %GroupID_2 = load i64, ptr %11
; CHECK: %12 = mul i64 %InternalLocalSize_0, %GroupID_0
; CHECK: %13 = add i64 %12, %GlobalOffset_0
; CHECK: %14 = mul i64 %InternalLocalSize_1, %GroupID_1
; CHECK: %15 = add i64 %14, %GlobalOffset_1
; CHECK: %16 = mul i64 %InternalLocalSize_2, %GroupID_2
; CHECK: %17 = add i64 %16, %GlobalOffset_2
; CHECK: %18 = insertvalue [4 x i64] undef, i64 %13, 0
; CHECK: %19 = insertvalue [4 x i64] %18, i64 %15, 1
; CHECK: %BaseGlbId = insertvalue [4 x i64] %19, i64 %17, 2
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @A}
!1 = !{!"int*", !"int", !"int*"}
!2 = !{ptr addrspace(1) null, i32 0, ptr addrspace(1) null }

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-34: WARNING: Instruction with empty DebugLoc in function A {{.*}}
; DEBUGIFY-NOT: WARNING
