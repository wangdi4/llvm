; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prevent-div-crashes,instcombine -S %s -o - | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, ptr addrspace(1) nocapture %res) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %div = sdiv <4 x i32> %x, <i32 -2, i32 0, i32 632, i32 0>
  store <4 x i32> %div, ptr addrspace(1) %res
  ret void
}

; replacing 0 with 1 in the divisor vector
; CHECK: 	sdiv <4 x i32> %x, <i32 -2, i32 1, i32 632, i32 1>

; DEBUGIFY-NOT: WARNING

!0 = !{!"int4", !"int4*"}
!1 = !{<4 x i32> zeroinitializer, ptr addrspace(1) null}
