; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i32 %x, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %div = udiv i32 %x, 0
  store i32 %div, ptr addrspace(1) %res
  ret void
}

; %div = udiv %x, 1 = %x --> udiv instruction should disappear
; CHECK-NOT: 	udiv i32 %x

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY-NOT: WARNING

!0 = !{!"int", !"int*"}
!1 = !{i32 0, ptr addrspace(1) null}
