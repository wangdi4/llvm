; RUN: opt -passes=auto-cpu-clone -acd-enable-all < %s -S | FileCheck %s

; This test checks that GlobalAliases aliasing a multi-versioned function, are also
; multi-versioned.
; This test also ensures that, the multiversioning of cascaded GlobalAliases,
; does not lead to auto-cpu-clone pass infinitely looping as in CMPLRLLVM-46752.


; CHECK: @_ZN3VertexObjectD2Ev.A = alias void (ptr), ptr @_ZN3ObjectD2Ev.A
; CHECK: @_ZN3VertexObjectD1Ev.A = alias void (ptr), ptr @_ZN3VertexObjectD2Ev.A
; CHECK: @_ZN3VertexObjectD2Ev = alias void (ptr), ptr @_ZN3ObjectD2Ev
; CHECK: @_ZN3VertexObjectD2Ev.V = alias void (ptr), ptr @_ZN3ObjectD2Ev.V
; CHECK: @_ZN3VertexObjectD1Ev = alias void (ptr), ptr @_ZN3VertexObjectD2Ev
; CHECK: @_ZN3VertexObjectD1Ev.V = alias void (ptr), ptr @_ZN3VertexObjectD2Ev.V

; CHECK: define void @_ZN3ObjectD2Ev.A() #0 !llvm.acd.clone !0 {
; CHECK: define void @_ZN3ObjectD2Ev.V() #1 !llvm.acd.clone !0 {


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZN3VertexObjectD2Ev = alias void (ptr), ptr @_ZN3ObjectD2Ev
@_ZN3VertexObjectD1Ev = alias void (ptr), ptr @_ZN3VertexObjectD2Ev

define void @_ZN3ObjectD2Ev() !llvm.auto.arch !0 {
  ret void
}

!0 = !{!"core-avx2"}
