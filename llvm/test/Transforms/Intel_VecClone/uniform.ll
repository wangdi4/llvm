; Check to make sure the initial parameter store of the uniform parameter is sunk into the loop.

; RUN: opt -vec-clone -S < %s | FileCheck %s

; CHECK-LABEL: <4 x i32> @_ZGVbN4u_foo(i32 %b)
; CHECK: simd.loop:
; CHECK: store i32 %b

; ModuleID = 'uniform.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %b) #0 {
entry:
  %b.addr = alloca i32, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32, i32* %b.addr, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %b.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  ret i32 %1
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4u_,_ZGVbN4u_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
