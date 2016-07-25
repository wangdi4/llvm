; RUN: opt -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(float %x, i64* %y) nounwind {
  %tmp = fptosi float %x to i64
  store i64 %tmp, i64* %y
  ret void
}

; CHECK: call i64 @_Z12convert_longf(float %x)
