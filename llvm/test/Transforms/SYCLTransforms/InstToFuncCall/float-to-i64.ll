; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(float %x, ptr %y) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %tmp = fptosi float %x to i64
  store i64 %tmp, ptr %y
  ret void
}

; CHECK: call i64 @_Z12convert_longf(float %x)

; DEBUGIFY-NOT: WARNING

!0 = !{!"float", !"long*"}
!1 = !{float 0.000000e+0, ptr null}
