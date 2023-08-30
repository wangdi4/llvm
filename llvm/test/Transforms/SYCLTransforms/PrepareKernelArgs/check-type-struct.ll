; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>

; CHECK: @t1
define void @t1(ptr byval(%struct.struct1) %arg0, ptr addrspace(1) %arg1) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  ret void
}

;; struct1 arg0 - packed struct - 17 bytes - byval (no load)
; CHECK: [[ARG0_BUFF_INDEX:%.*]] = getelementptr i8, ptr %UniformArgs, i32 0
;; struct1* arg1 - 4 bytes - expected alignment: 20
; CHECK: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, ptr %UniformArgs, i32 20
; CHECK-NEXT: %explicit_1 = load ptr addrspace(1), ptr [[ARG1_BUFF_INDEX]], align 4
;;implicit args

;; copy of byval arg
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 1 %explicit_0.ptr, ptr align 1 [[ARG0_BUFF_INDEX]], i64 17, i1 false)
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{ptr @t1}
!1 = !{!"%struct.struct1", !"%struct.struct1*"}
!2 = !{ptr null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-32: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
