; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(i64 %x, ptr %y) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %tmp = uitofp i64 %x to double
  store double %tmp, ptr %y
  ret void
}

; CHECK: call double @_Z14convert_doublem(i64 %x)

; DEBUGIFY-NOT: WARNING

!0 = !{!"unsigned long", !"double*"}
!1 = !{i64 0, ptr null}
