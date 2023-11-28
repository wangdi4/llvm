; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s

;; This is Win32 target, pointer size is 4 bytes
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>
%struct.struct2 = type <{ i32 ,i32 ,i32 }>

; CHECK: @t1
define void @t1(ptr addrspace(1) %arg0, ptr addrspace(1) %arg1, ptr byval(<4 x i32>) %arg2) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  ret void
}

; CHECK: %explicit_2.ptr = alloca <4 x i32>, align 16

;; struct1* arg0 - 4 bytes - expected alignment: 0
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 0
; CHECK: %explicit_0 = load ptr addrspace(1), ptr [[ARG0_BUFF_INDEX]], align 4
;; struct2* arg1 - 4 bytes - expected alignment: 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 4
; CHECK-NEXT: %explicit_1 = load ptr addrspace(1), ptr [[ARG1_BUFF_INDEX]], align 4
;; int4 byvalue arg2, size is the actual size not the pointer size - expected alignment:16 (4+12 = 16 so it's aligned to 16)
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%.*]] = getelementptr i8, ptr %UniformArgs, i32 16
;;implicit args

;; copy of byval arg
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 1 %explicit_2.ptr, ptr align 1 [[ARG2_BUFF_INDEX]], i64 16, i1 false)
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @t1}
!1 = !{!"%struct.struct1*", !"%struct.struct2*", !"int __attribute__((ext_vector_type(4)))"}
!2 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-34: WARNING: Instruction with empty DebugLoc in function t1 {{.*}}
; DEBUGIFY-NOT: WARNING
