; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This tests that the ModRef analysis for "printf"-like functions does not
; crash when it uses an external constant for the format string parameter.
; (CMPLRLLVM-39233)
;
; In this case, a conservative result should be returned because
; the constant string cannot be analyzed.

@nabout = internal local_unnamed_addr global ptr null

declare dso_local noalias ptr @malloc(i64)
declare dso_local i32 @fprintf(ptr nocapture, ptr nocapture readonly, ...)

@.str = external constant [13 x i8], align 1
define internal void @test01() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1, align 1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1, align 4
  %fp = load ptr, ptr @nabout, align 8
  %tmp = call i32 (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str, ptr %ar1)
  ret void
}

; CHECK-LABEL: Function: test01:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str, ptr %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str, ptr %ar1)
