; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x double> %x, <16 x i64>* %y) nounwind {
  %tmp = fptosi <16 x double> %x to <16 x i64>
  store <16 x i64> %tmp, <16 x i64> * %y
  ret void
}

; CHECK: call <16 x i64> @_Z14convert_long16Dv16_d(<16 x double> %x)


; DEBUGIFY-NOT: WARNING
