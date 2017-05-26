; Check to be sure that when Mem2Reg is on that all updates to instructions referring to the original
; parameter are updated correctly. When Mem2Reg is on, instructions will refer to the parameters
; directly and not through a load, which is why this is tested separately.

; Note: the LLVM IR used as input to this test has already had Mem2Reg applied to it, so no need to
; do that here.

; RUN: opt -vec-clone -S < %s | FileCheck %s

; Begin non-masked variant checking

; CHECK-LABEL: @_ZGVbN4vv_vec_sum
; CHECK: simd.loop:
; CHECK: %vec.i.cast.gep = getelementptr i32, i32* %vec.i.cast, i32 %index
; CHECK: %vec.i.elem = load i32, i32* %vec.i.cast.gep
; CHECK: %vec.j.cast.gep = getelementptr i32, i32* %vec.j.cast, i32 %index
; CHECK: %vec.j.elem = load i32, i32* %vec.j.cast.gep
; CHECK: %add = add nsw i32 %vec.i.elem, %vec.j.elem

; ModuleID = 'two_vec_sum.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @vec_sum(i32 %i, i32 %j) #0 {
entry:
  %add = add nsw i32 %i, %j
  ret i32 %add
}

attributes #0 = { nounwind readnone uwtable "vector-variants"="_ZGVbM4vv_,_ZGVbN4vv_" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
