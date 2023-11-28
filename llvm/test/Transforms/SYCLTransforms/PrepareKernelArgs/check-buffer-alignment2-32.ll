; RUN: opt -mtriple=i686 -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -mtriple=i686 -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{i8, i8, i8}>

; CHECK: @t1
define void @t1(ptr addrspace(1) %arg0, <2 x i8> %arg1, i32 %arg2, <8 x i16> %arg3, i8 %arg4, ptr addrspace(1) %arg5 ) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  ret void
}

;; new func - Win32
;;struct1* arg0 size: 4
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = load ptr addrspace(1), ptr [[ARG0_BUFF_INDEX]], align 4

;; char2 arg1 - alignment: 2 - in UniformArgs 0+4 = 4 is aligned to 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 4
; CHECK-NEXT: %explicit_1 = load <2 x i8>, ptr [[ARG1_BUFF_INDEX]], align 2

;; int arg2 - alignment: 4 - in UniformArgs 4+2=6 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 8
; CHECK-NEXT: %explicit_2 = load i32, ptr [[ARG2_BUFF_INDEX]], align 4

;; short8 arg3 - alignment: 16 - in UniformArgs 8+4=12 is aligned to 16
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 16
; CHECK-NEXT: %explicit_3 = load <8 x i16>, ptr [[ARG3_BUFF_INDEX]], align 16

;; char arg4 - alignment: 1 - in UniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 32
; CHECK-NEXT: %explicit_4 = load i8, ptr [[ARG4_BUFF_INDEX]], align 1

;; int* arg5 - alignment: 4 - in UniformArgs 32+1=33 is aligned to 36
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 36
; CHECK-NEXT: %explicit_5 = load ptr addrspace(1), ptr [[ARG5_BUFF_INDEX]], align 4
;;implicit args
; CHECK: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @t1}
!1 = !{!"%struct.struct1*", !"char __attribute__((ext_vector_type(2)))", !"int", !"short __attribute__((ext_vector_type(8)))", !"char", !"int*"}
!2 = !{ptr addrspace(1) null, <2 x i8> zeroinitializer, i32 0, <8 x i16> zeroinitializer, i8 0, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-40: WARNING: Instruction with empty DebugLoc in function t1 {{.*}}
; DEBUGIFY-NOT: WARNING
