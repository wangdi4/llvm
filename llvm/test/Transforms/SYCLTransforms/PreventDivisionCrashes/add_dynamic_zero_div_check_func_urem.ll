; RUN: opt -passes=sycl-kernel-prevent-div-crashes -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i32 %x, i32 %y, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %rem = urem i32 %x, %y
  store i32 %rem, ptr addrspace(1) %res
  ret void
}

; CHECK: 		[[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq i32 %y, 0
; CHECK-NEXT: 	[[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or i1 false, [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: 	[[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select i1 [[IS_DIVISOR_BAD]], i32 1, i32 %y
; CHECK-NEXT: 	urem i32 %x, [[NEW_DIVISOR]]


; DEBUGIFY-NOT: WARNING

!0 = !{!"int", !"int", !"int*"}
!1 = !{i32 0, i32 0, ptr addrspace(1) null}
