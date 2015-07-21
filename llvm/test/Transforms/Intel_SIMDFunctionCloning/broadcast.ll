; Check simple broadcast of constant integer. SIMDFunctionCloning should be able to just emit vector code for this case.

; RUN: opt -simd-function-cloning -S < %s | FileCheck %s

; CHECK-LABEL: __regcall <4 x i32> @_ZGVxN4_foo()
; CHECK: entry:
; CHECK: ret <4 x i32> <i32 99, i32 99, i32 99, i32 99>

; ModuleID = 'foo.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo() #0 {
entry:
  ret i32 99
}

attributes #0 = { nounwind uwtable "_ZGVxM4_" "_ZGVxN4_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
