; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, ptr %y) nounwind {
  %tmp = uitofp <16 x i64> %x to <16 x double>
  store <16 x double> %tmp, ptr %y
  ret void
}

; CHECK: call <16 x double> @_Z16convert_double16Dv16_m(<16 x i64> %x)

; DEBUGIFY-NOT: WARNING

!0 = !{!"ulong16", !"double16*"}
!1 = !{<16 x i64> zeroinitializer, ptr null}
