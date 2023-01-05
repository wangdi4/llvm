; Check that aligned parameters properly have their alignment propagated to
; uses which should observe that alignment. This includes:
; - Loads/stores
; - GEPs that preserve alignment (which may be lower than the arg align,
;                                 e.g. Align(64) + 32 * i => Align(32))

; RUN: opt -opaque-pointers -passes="vec-clone,alignment-from-assumptions" -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; omp declare simd aligned(a: 16) uniform(b) aligned(b: 32) linear(c: 32) aligned(c: 64)

; CHECK-LABEL: @_ZGVbN4va16ua32l32a64_foo(<4 x ptr> align 16 %a, ptr align 32 %b, ptr align 64 %c)
; CHECK-DAG: store ptr %b, ptr [[ALLOCA_B:%.*]], align 8
; CHECK-DAG: store ptr %c, ptr [[ALLOCA_C:%.*]], align 8
; CHECK-DAG: [[VEC_A:%.*]] = alloca <4 x ptr>
; CHECK:     store <4 x ptr> %a, ptr [[VEC_A]]

; CHECK-LABEL: simd.loop.preheader:
; CHECK-DAG: [[LOAD_B:%.*]] = load ptr, ptr [[ALLOCA_B]]
; CHECK-DAG: [[LOAD_C:%.*]] = load ptr, ptr [[ALLOCA_C]]

; CHECK-LABEL: simd.loop.header:
; CHECK: [[GEP_A:%.*]] = getelementptr ptr, ptr [[VEC_A]]
; CHECK: [[ELEM_A:%.*]] = load ptr, ptr [[GEP_A]]

; CHECK: {{%.*}} = load i32, ptr [[ELEM_A]], align 16
; CHECK: {{%.*}} = load i32, ptr [[LOAD_B]], align 32
; CHECK: [[GEP_C:%.*]] = getelementptr i8, ptr [[LOAD_C]]
; CHECK: {{%.*}} = load i32, ptr [[GEP_C]], align 32

; Function Attrs: nounwind uwtable
define i32 @foo(ptr %a, ptr %b, ptr %c) #0 {
entry:
  %0 = load i32, ptr %a, align 4
  %1 = load i32, ptr %b, align 4
  %2 = load i32, ptr %c, align 4
  %add1 = add nsw i32 %0, %1
  %add2 = add nsw i32 %add1, %2
  ret i32 %add2
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4va16ua32l32a64_foo" }
