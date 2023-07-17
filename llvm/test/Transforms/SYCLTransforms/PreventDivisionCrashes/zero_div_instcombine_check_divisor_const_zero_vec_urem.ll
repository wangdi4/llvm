; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %rem = urem <16 x i64> %x, zeroinitializer
  store <16 x i64> %rem, ptr addrspace(1) %res
  ret void
}

; %rem = urem %x, <1, 1, 1, ..., 1> = %x --> udiv instruction should disappear
; CHECK-NOT: 	urem <16 x i64> %x

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY-NOT: WARNING

!0 = !{!"long16", !"long16*"}
!1 = !{<16 x i64> zeroinitializer, ptr addrspace(1) null}
