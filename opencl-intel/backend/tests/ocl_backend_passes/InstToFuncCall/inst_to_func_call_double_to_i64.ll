; RUN: opt -mic-passes -inst-to-func-call -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(double %x, i64* %y) nounwind {
  %tmp = fptosi double %x to i64
  store i64 %tmp, i64* %y
  ret void
}

; CHECK: call i64 @_Z12convert_longd(double %x)
