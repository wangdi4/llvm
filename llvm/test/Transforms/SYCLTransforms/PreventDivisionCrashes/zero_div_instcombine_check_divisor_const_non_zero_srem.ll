; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i32 %x, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %rem = srem i32 %x, 2
  store i32 %rem, ptr addrspace(1) %res
  ret void
}

; CHECK: 	srem i32 %x, 2

; DEBUGIFY-NOT: WARNING

!0 = !{!"int", !"int*"}
!1 = !{i32 0, ptr addrspace(1) null}
