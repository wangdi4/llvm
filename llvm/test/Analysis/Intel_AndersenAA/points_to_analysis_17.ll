; CMPLRLLVM-9260: Verifies that points-to info for foo:%1 is not
; incorrectly computed as empty.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; Andersens Analysis shouldn't treat return value of indirect call as
; non-pointer when the return value is used by load instruction.

; CHECK: foo:%1        --> (0): <universal>
; CHECK-NOT: [0] foo:%1        -->

@fp = common global i32** (i32)* null, align 8

define void @foo(i32* nocapture readonly %inp) {
entry:
  %0 = load i32** (i32)*, i32** (i32)** @fp
  %call = tail call i32** %0(i32 2)
  %1 = load i32*, i32** %call
  ret void
}
