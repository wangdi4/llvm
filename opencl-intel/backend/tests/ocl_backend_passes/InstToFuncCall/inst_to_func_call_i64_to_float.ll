; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(i64 %x, float* %y) nounwind {
  %tmp = sitofp i64 %x to float
  store float %tmp, float* %y
  ret void
}

; CHECK: call float @_Z13convert_floatl(i64 %x)
