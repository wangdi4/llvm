; This test verifies that anders-aa should be able to handle
; freeze instruction without bailing out early.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: foo:freezemallocptr  --> (0): <universal>

define void  @foo() {
  %call1 = tail call noalias i8* @malloc(i64 40)
  %p1 = bitcast i8* %call1 to i32**
  %freezemallocptr = freeze i32** %p1
  ret void
}

declare dso_local i8* @malloc(i64)
