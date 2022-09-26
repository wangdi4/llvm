; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; vector stores.

; CHECK: DTRANS Weak Align: inhibited -- Unsupported StoreInst

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = global float* null, align 8

; IR corresponding to _mm_store_ps() intrinsic call. Calls
; to the intrinsic are directly lowered by the front-end.
define internal void @test01(i32 %elem) {
  %a_ptr = load float*, float** @a, align 8
  %idxprom = sext i32 %elem to i64
  %arrayidx = getelementptr inbounds float, float* %a_ptr, i64 %idxprom
  %vec_ptr = bitcast float* %arrayidx to <4 x float>*
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %vec_ptr, align 16
  ret void
}

define i32 @main() {
  call void @test01(i32 4)
  ret i32 0
}
