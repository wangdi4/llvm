; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @A(<4 x i8> %c, <4 x i8> %uc, <4 x i16> %s, <4 x i16> %us, <4 x i32> %i, <4 x i32> %ui, <4 x float> %f, ptr addrspace(1) nocapture %result) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  ret void
}

; CHECK: define void @A(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle)
; CHECK: %0 = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK: %explicit_0 = load <4 x i8>, ptr %0, align 4
; CHECK: %1 = getelementptr i8, ptr %UniformArgs, i32 4
; CHECK: %explicit_1 = load <4 x i8>, ptr %1, align 4
; CHECK: %2 = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK: %explicit_2 = load <4 x i16>, ptr %2, align 8
; CHECK: %3 = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK: %explicit_3 = load <4 x i16>, ptr %3, align 8
; CHECK: %4 = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK: %explicit_4 = load <4 x i32>, ptr %4, align 16
; CHECK: %5 = getelementptr i8, ptr %UniformArgs, i32 48
; CHECK: %explicit_5 = load <4 x i32>, ptr %5, align 16
; CHECK: %6 = getelementptr i8, ptr %UniformArgs, i32 64
; CHECK: %explicit_6 = load <4 x float>, ptr %6, align 16
; CHECK: %7 = getelementptr i8, ptr %UniformArgs, i32 80
; CHECK: %explicit_7 = load ptr addrspace(1), ptr %7, align 8
; CHECK: %pWorkDim = getelementptr i8, ptr %UniformArgs, i32 88
; CHECK: %8 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i64, ptr %8
; CHECK: %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i64, ptr %9
; CHECK: %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i64, ptr %10
; CHECK: %11 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64, ptr %11
; CHECK: %12 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64, ptr %12
; CHECK: %13 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64, ptr %13
; CHECK: %14 = getelementptr i64, ptr %pWGId, i32 0
; CHECK: %GroupID_0 = load i64, ptr %14
; CHECK: %15 = getelementptr i64, ptr %pWGId, i32 1
; CHECK: %GroupID_1 = load i64, ptr %15
; CHECK: %16 = getelementptr i64, ptr %pWGId, i32 2
; CHECK: %GroupID_2 = load i64, ptr %16
; CHECK: %17 = mul i64 %InternalLocalSize_0, %GroupID_0
; CHECK: %18 = add i64 %17, %GlobalOffset_0
; CHECK: %19 = mul i64 %InternalLocalSize_1, %GroupID_1
; CHECK: %20 = add i64 %19, %GlobalOffset_1
; CHECK: %21 = mul i64 %InternalLocalSize_2, %GroupID_2
; CHECK: %22 = add i64 %21, %GlobalOffset_2
; CHECK: %23 = insertvalue [4 x i64] undef, i64 %18, 0
; CHECK: %24 = insertvalue [4 x i64] %23, i64 %20, 1
; CHECK: %BaseGlbId = insertvalue [4 x i64] %24, i64 %22, 2
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @A}
!1 = !{!"char __attribute__((ext_vector_type(4)))", !"uchar __attribute__((ext_vector_type(4)))", !"short __attribute__((ext_vector_type(4)))", !"ushort __attribute__((ext_vector_type(4)))", !"int __attribute__((ext_vector_type(4)))", !"uint __attribute__((ext_vector_type(4)))", !"float __attribute__((ext_vector_type(4)))", !"float __attribute__((ext_vector_type(4)))*"}
!2 = !{<4 x i8> zeroinitializer, <4 x i8> zeroinitializer, <4 x i16> zeroinitializer, <4 x i16> zeroinitializer, <4 x i32> zeroinitializer, <4 x i32> zeroinitializer, <4 x float> zeroinitializer, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-44: WARNING: Instruction with empty DebugLoc in function A {{.*}}
; DEBUGIFY-NOT: WARNING
