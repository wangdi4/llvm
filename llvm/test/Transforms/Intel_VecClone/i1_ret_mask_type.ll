; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define zeroext i1 @_Z3fooj(i32 %i) #0 {
; CHECK:define zeroext <16 x i8> @_ZGVbM16v__Z3fooj(<16 x i32> [[I0:%.*]], <16 x i8> [[MASK:%.*]]) #1 { 
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_I0:%.*]] = alloca <16 x i32>, align 64
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <16 x i8>, align 16
; CHECK-NEXT:    [[VEC_RETVAL0:%.*]] = alloca <16 x i8>, align 16
; CHECK-NEXT:    store <16 x i32> [[I0]], ptr [[VEC_I0]], align 64
; CHECK-NEXT:    store <16 x i8> [[MASK]], ptr [[VEC_MASK]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0:%.*]]
;

entry:
  %cmp = icmp eq i32 %i, 97
  ret i1 %cmp
}

attributes #0 = { noinline nounwind "vector-variants"="_ZGVbM16v__Z3fooj" }
