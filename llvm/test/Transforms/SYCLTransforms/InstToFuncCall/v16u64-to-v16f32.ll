; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, ptr %y) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %tmp = uitofp <16 x i64> %x to <16 x float>
  store <16 x float> %tmp, ptr %y
  ret void
}

; CHECK: call <16 x float> @_Z15convert_float16Dv16_m(<16 x i64> %x)

; DEBUGIFY-NOT: WARNING

!0 = !{!"ulong16", !"float16*"}
!1 = !{<16 x i64> zeroinitializer, ptr null}
