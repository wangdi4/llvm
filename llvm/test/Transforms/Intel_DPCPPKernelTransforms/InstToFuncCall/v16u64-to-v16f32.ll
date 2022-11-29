; RUN: opt -passes=dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-inst-to-func-call -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, <16 x float>* %y) nounwind {
  %tmp = uitofp <16 x i64> %x to <16 x float>
  store <16 x float> %tmp, <16 x float> * %y
  ret void
}

; CHECK: call <16 x float> @_Z15convert_float16Dv16_m(<16 x i64> %x)

; DEBUGIFY-NOT: WARNING
