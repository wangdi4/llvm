; Check to see that the linear parameter i is updated with the correct stride, indicated by a mul/add instruction sequence after the load.

; RUN: opt -simd-function-cloning -S < %s | FileCheck %s

; CHECK-LABEL: __regcall <4 x i32> @_ZGVxN4lu_vec_func(i32 %i, i32 %x)
; CHECK: simd.loop:
; CHECK: %0 = load i32, i32* %i.addr
; CHECK: %mul = mul i32 1, %index
; CHECK: %add.1 = add i32 %0, %mul

; ModuleID = 'linear.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @vec_func(i32 %i, i32 %x) #0 {
entry:
  %i.addr = alloca i32, align 4
  %x.addr = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, 4
  store i32 %add, i32* %y, align 4
  %1 = load i32, i32* %y, align 4
  ret i32 %1
}

attributes #0 = { nounwind uwtable "_ZGVxM4lu_" "_ZGVxN4lu_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
