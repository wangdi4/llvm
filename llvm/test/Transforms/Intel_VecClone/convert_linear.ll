; Check handling of upconverting a linear (variable %i) to ensure stride calculation
; is inserted correctly and the old convert (sext) uses the stride instead of the old
; reference to %i.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN2vl_foo
; CHECK: simd.begin.region:
; CHECK: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: DIR.OMP.SIMD
; CHECK-SAME: QUAL.OMP.SIMDLEN
; CHECK-SAME: i32 2
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %i.addr)
; CHECK: simd.loop.header:
; CHECK: %0 = load i32, i32* %i.addr
; CHECK-NEXT: %stride.mul = mul i32 1, %index
; CHECK-NEXT: %stride.add = add i32 %0, %stride.mul
; CHECK-NEXT: %conv = sext i32 %stride.add to i64

; ModuleID = 'convert.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i64 @foo(i64 %x, i32 %i) #0 {
entry:
  %x.addr = alloca i64, align 8
  %i.addr = alloca i32, align 4
  store i64 %x, i64* %x.addr, align 8
  store i32 %i, i32* %i.addr, align 4
  %0 = load i32, i32* %i.addr, align 4
  %conv = sext i32 %0 to i64
  %1 = load i64, i64* %x.addr, align 8
  %add = add nsw i64 %conv, %1
  ret i64 %add
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbM2vl_,_ZGVbN2vl_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
