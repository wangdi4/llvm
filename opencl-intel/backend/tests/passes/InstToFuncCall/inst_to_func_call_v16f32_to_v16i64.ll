; RUN: %oclopt -inst-to-func-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<16 x float> %x, <16 x i64>* %y) nounwind {
  %tmp = fptosi <16 x float> %x to <16 x i64>
  store <16 x i64> %tmp, <16 x i64> * %y
  ret void
}

; CHECK: call <16 x i64> @_Z14convert_long16Dv16_f(<16 x float> %x)


; DEBUGIFY-NOT: WARNING
