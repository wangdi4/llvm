; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -predicate -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @main
define void @main(float* %ptr0, float* %ptr1, i1 %pred) {
; CHECK: @printf
; CHECK-NOT: @masked
  %f = call float @printf(i1 %pred, float* %ptr0)
  ret void
; CHECK: ret
}

declare float @printf(i1 %pred, float* %ptr0)


