; CMPLRLLVM-9260: Verifies that points-to info for foo:%1 is not
; incorrectly computed as empty.
; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; Andersens Analysis shouldn't treat return value of indirect call as
; non-pointer when the return value is used by load instruction.

; CHECK: foo:%1        --> (0): <universal>
; CHECK-NOT: [0] foo:%1        -->

@fp = common global ptr null, align 8

define void @foo(ptr nocapture readonly %inp) {
entry:
  %0 = load ptr, ptr @fp
  %call = tail call ptr %0(i32 2)
  %1 = load ptr, ptr %call
  ret void
}
