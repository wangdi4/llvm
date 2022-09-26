; Check to make sure that 'may-have-openmp-directive' appears after VecClone.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: <4 x i32> @_ZGVbN4u_foo(i32 %b) #1
; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; Note: A new alloca for the uniform is created (%alloca.b), it is marked as
; uniform, and the argument (%b) is stored to it. In the loop preheader this
; value is loaded (%load.b) and use-def chains are updated, which includes
; the original store of %b. Since this store is inside the loop it is marked
; as private.
; CHECK-SAME: QUAL.OMP.UNIFORM
; CHECK-SAME: i32* %alloca.b
; CHECK-SAME: QUAL.OMP.PRIVATE
; CHECK-SAME: i32* %b.addr
; CHECK: simd.loop.header:
; CHECK: store i32 %load.b

; CHECK: attributes #1
; CHECK-SAME: "may-have-openmp-directive"="true"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

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

attributes #0 = { optnone noinline nounwind uwtable "vector-variants"="_ZGVbM4u_foo,_ZGVbN4u_foo" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
