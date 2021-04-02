; RUN: %oclopt -inst-to-func-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(i64 %x, float* %y) nounwind {
  %tmp = uitofp i64 %x to float
  store float %tmp, float* %y
  ret void
}

; CHECK: call float @_Z13convert_floatm(i64 %x)

; DEBUGIFY-NOT: WARNING
