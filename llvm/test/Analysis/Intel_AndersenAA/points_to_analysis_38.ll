; This test verifies that "store i32* null, i32** %p2" is NOT removed by
; instcombine using Andersens's points-to analysis. This test
; verifies that InlineAsm calls are treated conservatively.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: define i32 @foo(
; CHECK: store

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, i32* }

define dso_local %struct.S* @bar(%struct.S* noundef %p) local_unnamed_addr {
entry:
  %0 = tail call %struct.S* asm sideeffect "", "=r,0,~{memory},~{dirflag},~{fpsr},~{flags}"(%struct.S* %p)
  ret %struct.S* %0
}

define i32 @foo(i32* %q) {
entry:
  %call = call %struct.S* @bar(%struct.S* null)
  %p2 = getelementptr inbounds %struct.S, %struct.S* %call, i64 0, i32 1
  store i32* null, i32** %p2, align 8
  ret i32 0
}
