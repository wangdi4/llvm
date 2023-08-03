; This test verifies that anders-aa should be able to handle
; freeze instruction without bailing out early.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: foo:freezemallocptr  --> (0): <universal>

define void  @foo() {
  %call1 = tail call noalias ptr @malloc(i64 40)
  %p1 = bitcast ptr %call1 to ptr
  %freezemallocptr = freeze ptr %p1
  ret void
}

declare dso_local ptr @malloc(i64)
