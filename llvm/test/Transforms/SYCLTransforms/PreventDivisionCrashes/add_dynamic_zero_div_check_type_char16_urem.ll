; RUN: opt -passes=sycl-kernel-prevent-div-crashes -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i8> %x, <16 x i8> %y, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %rem = urem <16 x i8> %x, %y
  store <16 x i8> %rem, ptr addrspace(1) %res
  ret void
}

; CHECK: 		[[IS_DIVISOR_ZERO:%[a-zA-Z0-9]+]] = icmp eq <16 x i8> %y, zeroinitializer
; CHECK: 		[[IS_DIVISOR_BAD:%[a-zA-Z0-9]+]] = or <16 x i1> zeroinitializer, [[IS_DIVISOR_ZERO]]
; CHECK-NEXT: 	[[NEW_DIVISOR:%[a-zA-Z0-9]+]] = select <16 x i1> [[IS_DIVISOR_BAD]], <16 x i8> <i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1>, <16 x i8> %y
; CHECK-NEXT: 	urem <16 x i8> %x, [[NEW_DIVISOR]]


; DEBUGIFY-NOT: WARNING

!0 = !{!"char16", !"char16", !"char16*"}
!1 = !{<16 x i8> zeroinitializer, <16 x i8> zeroinitializer, ptr addrspace(1) null}
