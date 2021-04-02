; RUN: %oclopt -inst-to-func-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<16 x i64> %x, <16 x double>* %y) nounwind {
  %tmp = uitofp <16 x i64> %x to <16 x double>
  store <16 x double> %tmp, <16 x double> * %y
  ret void
}

; CHECK: call <16 x double> @_Z16convert_double16Dv16_m(<16 x i64> %x)

; DEBUGIFY-NOT: WARNING
