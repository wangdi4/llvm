; Check to make sure that 'may-have-openmp-directive' appears after VecClone.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: <4 x i32> @_ZGVbN4u_foo(i32 %b) #1
; CHECK: simd.begin.region:
; CHECK-NEXT: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 4
; FIXME: alloca for %b should be marked as uniform. This is temporary because
; this will be fixed as part of CMPLRLLVM-9851. This is just a side-effect
; from this refactor.
; CHECK-SAME: QUAL.OMP.PRIVATE
; CHECK-SAME: i32* %b.addr
; CHECK: simd.loop.header:
; CHECK: store i32 %b

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

attributes #0 = { optnone noinline nounwind uwtable "vector-variants"="_ZGVbM4u_,_ZGVbN4u_" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
