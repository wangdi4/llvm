; RUN: %oclopt -inst-to-func-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(i64 %x, double* %y) nounwind {
  %tmp = sitofp i64 %x to double
  store double %tmp, double * %y
  ret void
}

; CHECK: call double @_Z14convert_doublel(i64 %x)

; DEBUGIFY-NOT: WARNING
