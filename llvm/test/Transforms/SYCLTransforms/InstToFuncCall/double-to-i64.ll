; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(double %x, ptr %y) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %tmp = fptosi double %x to i64
  store i64 %tmp, ptr %y
  ret void
}

; CHECK: call i64 @_Z12convert_longd(double %x) [[ATTR:#[0-9]+]]

; CHECK: [[ATTR]] = { nounwind willreturn memory(none) }

; DEBUGIFY-NOT: WARNING

!0 = !{!"double", !"long*"}
!1 = !{double 0.000000e+0, ptr null}
