; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK-LABEL: @sample_test
define void @sample_test(double %x, half* %y) nounwind {
  %tmp = fptrunc double %x to half
  store half %tmp, half* %y
  ret void
}

; CHECK: call half @convert_halfd(double %x)
