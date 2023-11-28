; Check to see that we properly promote boolean (`i1` or `<N x i1>`) parameters
; (to `<VF x i8>` or `<VF x N x i8>`) to avoid generating a GEP into a
; `<VF x i1>` when expanding argument uses.
; Additionally, check that we properly truncate the value back to `i1` or
; `<N x i1>` before uses so that users observe the correct type.

; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN16v_foo(<16 x i8> %p)
; CHECK: entry:
; CHECK:   [[VEC_P:%.*]] = alloca <16 x i8>

; CHECK: simd.loop.header:
; CHECK:   [[GEP:%.*]] = getelementptr i8, ptr [[VEC_P]], i32 [[INDEX:%.*]]
; CHECK:   [[ELEM:%.*]] = load i8, ptr [[GEP]]
; CHECK:   [[TRUNC:%.*]] = trunc i8 [[ELEM]] to i1
; CHECK:   {{%.*}} = select i1 [[TRUNC]], i32 48858, i32 64206

; CHECK-LABEL: @_ZGVbM16v_foo(<16 x i8> %p, <16 x i32> %mask)
; CHECK: entry:
; CHECK:   [[VEC_P:%.*]] = alloca <16 x i8>

; CHECK: simd.loop.header:
; CHECK:   [[GEP:%.*]] = getelementptr i8, ptr [[VEC_P]], i32 [[INDEX:%.*]]
; CHECK:   [[ELEM:%.*]] = load i8, ptr [[GEP]]
; CHECK:   [[TRUNC:%.*]] = trunc i8 [[ELEM]] to i1
; CHECK:   {{%.*}} = select i1 [[TRUNC]], i32 48858, i32 64206

; CHECK-LABEL: @_ZGVbN16v_foo_vec(<32 x i8> %p)
; CHECK: entry:
; CHECK:   [[VEC_P:%.*]] = alloca <32 x i8>

; CHECK: simd.loop.header:
; CHECK:   [[GEP:%.*]] = getelementptr <2 x i8>, ptr [[VEC_P]], i32 [[INDEX:%.*]]
; CHECK:   [[ELEM:%.*]] = load <2 x i8>, ptr [[GEP]]
; CHECK:   [[TRUNC:%.*]] = trunc <2 x i8> [[ELEM]] to <2 x i1>
; CHECK:   {{%.*}} = select <2 x i1> [[TRUNC]], <2 x i32> <i32 48858, i32 48858>, <2 x i32> <i32 64206, i32 64206>

; CHECK-LABEL: @_ZGVbM16v_foo_vec(<32 x i8> %p, <16 x i32> %mask)
; CHECK: entry:
; CHECK:   [[VEC_P:%.*]] = alloca <32 x i8>

; CHECK: simd.loop.header:
; CHECK:   [[GEP:%.*]] = getelementptr <2 x i8>, ptr [[VEC_P]], i32 [[INDEX:%.*]]
; CHECK:   [[ELEM:%.*]] = load <2 x i8>, ptr [[GEP]]
; CHECK:   [[TRUNC:%.*]] = trunc <2 x i8> [[ELEM]] to <2 x i1>
; CHECK:   {{%.*}} = select <2 x i1> [[TRUNC]], <2 x i32> <i32 48858, i32 48858>, <2 x i32> <i32 64206, i32 64206>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i1 %p) #0 {
entry:
  %cond = select i1 %p, i32 48858, i32 64206
  ret i32 %cond
}

define <2 x i32> @foo_vec(<2 x i1> %p) #1 {
entry:
  %cond = select <2 x i1> %p, <2 x i32> <i32 48858, i32 48858>, <2 x i32> <i32 64206, i32 64206>
  ret <2 x i32> %cond
}

attributes #0 = { "vector-variants"="_ZGVbN16v_foo,_ZGVbM16v_foo" }
attributes #1 = { "vector-variants"="_ZGVbN16v_foo_vec,_ZGVbM16v_foo_vec" }
