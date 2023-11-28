; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x double> %x, ptr %y) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %tmp = fptosi <16 x double> %x to <16 x i64>
  store <16 x i64> %tmp, ptr %y
  ret void
}

; CHECK: call <16 x i64> @_Z14convert_long16Dv16_d(<16 x double> %x)


; DEBUGIFY-NOT: WARNING

!0 = !{!"double16", !"long16*"}
!1 = !{<16 x double> zeroinitializer, ptr null}
