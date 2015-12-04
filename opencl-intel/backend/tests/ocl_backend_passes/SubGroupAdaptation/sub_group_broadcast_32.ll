; RUN: opt -sub-group-adaptation -verify -S < %s | FileCheck %s
; ModuleID = 'Program'
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"

; CHECK: @sw_broadcast_test
; CHECK: entry
; CHECK-NOT: call spir_func i32 @_Z22get_sub_group_local_idv()
; CHECK: call spir_func i32 @_Z19get_local_linear_idv()
; CHECK: call spir_func i32 @_Z19sub_group_broadcastij(i32 %2, i32 %3)
; CHECK: call spir_func i32 @_Z19sub_group_broadcastij(i32 %6, i32 %7)
; CHECK: define spir_func i32 @_Z19sub_group_broadcastij(i32, i32)
; CHECK: entry
; CHECK: %lsz0 = call spir_func i32 @_Z14get_local_sizej(i32 0)
; CHECK: %lsz1 = call spir_func i32 @_Z14get_local_sizej(i32 1)
; CHECK: %lid0 = call spir_func i32 @_Z12get_local_idj(i32 0)
; CHECK: %lid1 = call spir_func i32 @_Z12get_local_idj(i32 1)
; CHECK: %lid2 = call spir_func i32 @_Z12get_local_idj(i32 2)
; CHECK: %lid0.res = urem i32 %0, %lsz0
; CHECK: %lid1.op = udiv i32 %0, %lsz0
; CHECK: %lid1.res = urem i32 %lid1.op, %lsz1
; CHECK: %lid2.op = sub i32 %lid1.op, %lid1.res
; CHECK: %lid2.res = udiv i32 %lid2.op, %lsz1
; CHECK: %CallWGBroadCast = call i32 @_Z20work_group_broadcastijjj(i32 %0, i32 %lid0.res, i32 %lid1.res, i32 %lid2.res)
; CHECK: ret i32 %0


; Function Attrs: nounwind
define spir_kernel void @sw_broadcast_test(i32 %id, i32 addrspace(1)* %b) #0 {
entry:
  %id.addr = alloca i32, align 4
  %b.addr = alloca i32 addrspace(1)*, align 4
  %sglid = alloca i32, align 4
  %res = alloca i32, align 4
  %sgid = alloca i32, align 4
  store i32 %id, i32* %id.addr, align 4
  store i32 addrspace(1)* %b, i32 addrspace(1)** %b.addr, align 4
  %call = call spir_func i32 @_Z22get_sub_group_local_idv()
  store i32 %call, i32* %sglid, align 4
  %0 = load i32* %sglid, align 4
  %1 = load i32 addrspace(1)** %b.addr, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %1, i32 %0
  %2 = load i32 addrspace(1)* %arrayidx, align 4
  %3 = load i32* %id.addr, align 4
  %call1 = call spir_func i32 @_Z19sub_group_broadcastij(i32 %2, i32 %3)
  store i32 %call1, i32* %res, align 4
  %call2 = call spir_func i32 @_Z16get_sub_group_idv()
  store i32 %call2, i32* %sgid, align 4
  %4 = load i32* %sgid, align 4
  %5 = load i32 addrspace(1)** %b.addr, align 4
  %arrayidx3 = getelementptr inbounds i32 addrspace(1)* %5, i32 %4
  %6 = load i32 addrspace(1)* %arrayidx3, align 4
  %7 = load i32* %id.addr, align 4
  %call4 = call spir_func i32 @_Z19sub_group_broadcastij(i32 %6, i32 %7)
  store i32 %call4, i32* %res, align 4
  ret void
}

declare spir_func i32 @_Z22get_sub_group_local_idv() #1

declare spir_func i32 @_Z19sub_group_broadcastij(i32, i32) #1

declare spir_func i32 @_Z16get_sub_group_idv() #1

; CHECK-NOT: declare spir_func i32 @_Z22get_sub_group_local_idv()
; CHECK: declare spir_func i32 @_Z19get_local_linear_idv()
; CHECK: declare spir_func i32 @_Z14get_local_sizej(i32)
; CHECK: declare spir_func i32 @_Z12get_local_idj(i32)
; CHECK: declare spir_func i32 @_Z20work_group_broadcastijjj(i32, i32, i32, i32)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i32, i32 addrspace(1)*)* @sw_broadcast_test, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int", !"int*"}
!5 = !{!"kernel_arg_type_qual", !"", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__attribute__((overloadable)) uint get_sub_group_local_id();
;;;__attribute__((overloadable)) uint get_sub_group_id();
;;;__attribute__((overloadable)) int sub_group_broadcast(int, uint);
;;;
;;;__kernel void sw_broadcast_test(int id, __global int* b) {
;;;  uint sglid = get_sub_group_local_id();
;;;  int res = sub_group_broadcast(b[sglid], id);
;;;  uint sgid = get_sub_group_id();
;;;  res = sub_group_broadcast(b[sgid], id);
;;;}