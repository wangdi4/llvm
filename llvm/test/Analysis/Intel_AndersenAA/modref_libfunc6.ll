; RUN: opt -opaque-pointers=0 < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This tests that the ModRef analysis for "printf"-like functions does not
; crash when it uses an external constant for the format string parameter.
; (CMPLRLLVM-39233)
;
; In this case, a conservative result should be returned because
; the constant string cannot be analyzed.

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@nabout = internal dso_local local_unnamed_addr global %struct._IO_FILE* null

declare dso_local noalias i8* @malloc(i64)
declare dso_local i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...)

; Test with library call to fprintf, passing pointer argument, and external constant string.
@.str = external constant [13 x i8], align 1
define internal void @test01() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test01:
 ;CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)
