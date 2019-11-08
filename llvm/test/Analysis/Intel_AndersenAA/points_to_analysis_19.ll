; CMPLRLLVM-9389: Andersens analysis shouldn't crash when constant
; extractelement appears in IR. Verify that points-to set of return
; value of foo is conservatively computed as UniversalSet.

; RUN: opt < %s -anders-aa -print-anders-points-to -disable-output  2>&1 | FileCheck %s

; CHECK: [1] foo:retval     --> (0): <universal>

@GLOBAL = internal global i32 zeroinitializer

define i32* @foo() {
entry:
  %gep = getelementptr i32, <2 x i32*> <i32* @GLOBAL, i32* @GLOBAL>, <2 x i64> <i64 0, i64 1>
  %ext = extractelement <2 x i32*> %gep, i32 1
  ret i32* %ext
}

; Makes sure Andersens analysis shouldn't crash when constant
; extractelement appears in IR.
define i32* @bar() {
entry:
ret i32* extractelement (<2 x i32*> getelementptr (i32, <2 x i32*> <i32* @GLOBAL, i32* @GLOBAL>, <2 x i64> <i64 0, i64 1>), i32 1)
}
