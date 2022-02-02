; Check to see that the linear parameter i is updated with the correct stride.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4lu_foo

; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; CHECK-SAME: QUAL.OMP.PRIVATE
; CHECK-SAME: i32* %i.addr
; CHECK-SAME: QUAL.OMP.PRIVATE
; CHECK-SAME: i32* %x.addr
; CHECK-NEXT: br label %simd.loop

; CHECK: simd.loop.header:
; CHECK: %stride.mul = mul i32 1, %index
; CHECK-NEXT: %stride.add = add i32 %load.i, %stride.mul
; CHECK-NEXT: store i32 %stride.add, i32* %i.addr

; CHECK: simd.end.region:
; CHECK-NEXT: call void @llvm.directive.region.exit(token %entry.region)
; CHECK-SAME: DIR.OMP.END.SIMD
; CHECK-NEXT: br label %return

; ModuleID = 'linear.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %i, i32 %x) #0 {
entry:
  %i.addr = alloca i32, align 4
  %x.addr = alloca i32, align 4
  store i32 %i, i32* %i.addr, align 4
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %1 = load i32, i32* %i.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4lu_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
