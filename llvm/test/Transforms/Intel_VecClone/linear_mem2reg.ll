; Check to see that the linear parameter i is updated with the correct stride when Mem2Reg is on.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4lu_foo
; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.UNIFORM
; CHECK-SAME: i32* %alloca.x
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32* %alloca.i
; CHECK-SAME: i32 1
; CHECK: simd.loop:
; CHECK: %stride.mul = mul i32 1, %index
; CHECK-NEXT: %stride.add = add i32 %load.i, %stride.mul
; CHECK-NEXT: %add = add nsw i32 %load.x, %stride.add

;ModuleID = 'linear.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %i, i32 %x) #0 {
entry:
  %add = add nsw i32 %x, %i
  ret i32 %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM4lu_,_ZGVbN4lu_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
