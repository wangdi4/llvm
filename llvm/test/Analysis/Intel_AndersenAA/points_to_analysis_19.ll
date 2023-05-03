; CMPLRLLVM-9389: Andersens analysis shouldn't crash when constant
; extractelement appears in IR. Verify that points-to set of return
; value of foo is conservatively computed as UniversalSet.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output  2>&1 | FileCheck %s

; CHECK: [1] foo:retval     --> (0): <universal>

@GLOBAL = internal global i32 zeroinitializer

define ptr @foo() {
entry:
  %gep = getelementptr i32, <2 x ptr> <ptr @GLOBAL, ptr @GLOBAL>, <2 x i64> <i64 0, i64 1>
  %ext = extractelement <2 x ptr> %gep, i32 1
  ret ptr %ext
}

; Makes sure Andersens analysis shouldn't crash when constant
; extractelement appears in IR.
define ptr @bar() {
entry:
ret ptr extractelement (<2 x ptr> getelementptr (i32, <2 x ptr> <ptr @GLOBAL, ptr @GLOBAL>, <2 x i64> <i64 0, i64 1>), i32 1)
}
