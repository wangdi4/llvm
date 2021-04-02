; RUN: %oclopt -inst-to-func-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, <16 x float>* %y) nounwind {
  %tmp = sitofp <16 x i64> %x to <16 x float>
  store <16 x float> %tmp, <16 x float> * %y
  ret void
}

; CHECK: call <16 x float> @_Z15convert_float16Dv16_l(<16 x i64> %x)

; DEBUGIFY-NOT: WARNING
