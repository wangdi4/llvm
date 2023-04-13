; Check to see that aligned parameters are properly marked with the 'align'
; attr in cloned variants, and that alignment assumptions are inserted in the
; appropriate location:
;  - After the load in the preheader, for scalar (uniform/linear) arguments
;  - After arg uses in the loop body, for vector arguments

; Additionally, check that the inserted assumption are marked with metadata so
; downstream passes (VPlan) can clean them up after they are propagated.

; RUN: opt -opaque-pointers=0 -passes="vec-clone" -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @_ZGVbN4va16ua32l4a64_foo(<4 x i32*> align 16 %a, i32* align 32 %b, i32* align 64 %c)
; CHECK-DAG: store i32* %b, i32** [[ALLOCA_B:%.*]], align 8
; CHECK-DAG: store i32* %c, i32** [[ALLOCA_C:%.*]], align 8
; CHECK-DAG: [[VEC_A:%.*]] = alloca <4 x i32*>
; CHECK: [[CAST_A:%.*]] = bitcast <4 x i32*>* [[VEC_A]] to i32**
; CHECK: store <4 x i32*> %a, <4 x i32*>* [[VEC_A]]

; CHECK-LABEL: simd.loop.preheader:
; CHECK-DAG: [[LOAD_B:%.*]] = load i32*, i32** [[ALLOCA_B]], align 8
; CHECK-DAG: call void @llvm.assume(i1 true) [ "align"(i32* [[LOAD_B]], i64 32) ], !intel.vecclone.align.assume
; CHECK-DAG: [[LOAD_C:%.*]] = load i32*, i32** [[ALLOCA_C]], align 8
; CHECK-DAG: call void @llvm.assume(i1 true) [ "align"(i32* [[LOAD_C]], i64 64) ], !intel.vecclone.align.assume

; CHECK-LABEL: simd.loop.header:
; CHECK: [[GEP_A:%.*]] = getelementptr i32*, i32** [[CAST_A]]
; CHECK: [[A_ELEM:%.*]] = load i32*, i32** [[GEP_A]]
; CHECK: call void @llvm.assume(i1 true) [ "align"(i32* [[A_ELEM]], i64 16) ], !intel.vecclone.align.assume

; CHECK-LABEL: @_ZGVbM4va16ua32l4a64_foo(<4 x i32*> align 16 %a, i32* align 32 %b, i32* align 64 %c, <4 x i32> %mask)

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %a, i32* %b, i32* %c) #0 {
entry:
  %a.addr = alloca i32*, align 4
  %b.addr = alloca i32*, align 4
  %c.addr = alloca i32*, align 4
  store i32* %a, i32** %a.addr, align 4
  store i32* %b, i32** %b.addr, align 4
  store i32* %c, i32** %c.addr, align 4
  %0 = load i32*, i32** %a.addr, align 4
  %1 = load i32*, i32** %b.addr, align 4
  %2 = load i32*, i32** %c.addr, align 4
  %3 = load i32, i32* %0, align 4
  %4 = load i32, i32* %1, align 4
  %5 = load i32, i32* %2, align 4
  %add1 = add nsw i32 %3, %4
  %add2 = add nsw i32 %add1, %5
  ret i32 %add2
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4va16ua32l4a64_foo,_ZGVbM4va16ua32l4a64_foo" }
