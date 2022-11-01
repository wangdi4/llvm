; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This tests the ModRef analysis for various forms of printf routines.

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@nabout = internal dso_local local_unnamed_addr global %struct._IO_FILE* null

; Test with library call to fprintf, passing pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str = private constant [13 x i8] c"Result = %p\0A\00", align 1
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
; CHECK: Just Ref:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)

; Test with library call to fprintf, passing pointer argument, and non-constant string
@.str2 = internal global [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test02() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test02:
 ;CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0), i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0), i32* %ar1)

; The with library call using a format string that is not a compile time string
@.str3 = private constant [4 x i8] c"%s\0A\00", align 1
define internal void @test03() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1
  %str = call i8* @gets(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str3, i64 0, i64 0))
  %ld.str = load i8, i8* %str

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* %str, i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test03:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* %str, i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* %str, i32* %ar1)

; Test with library call using a format string that modifies the memory of the parameter
@.str4 = internal global [13 x i8] c"Set val--%n\0A\00", align 1
define internal void @test04() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1
  %arindex5 = getelementptr i32, i32* %ar1, i64 4
  %ld.arindex5 = load i32, i32* %arindex5

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0),
    i32* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test04:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)
; CHECK:   Both ModRef:  Ptr: i32* %arindex5	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)

; Test with library call using a format string that modifies the memory of the parameter and
; the %n has a length modifier.
@.str5 = internal global [13 x i8] c"Set val-%hn\0A\00", align 1
define internal void @test05() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 512)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i16*
  %ld.ar1 = load i16, i16* %ar1
  %arindex5 = getelementptr i16, i16* %ar1, i64 4
  %ld.arindex5 = load i16, i16* %arindex5

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0),
    i16* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test05:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %arindex5	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)

; Test with library call to printf (format string is 1st argument), passing
; pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str6 = private constant [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test06() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %tmp = call i32 (i8*, ...) @printf(
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test06:
; CHECK: Just Ref:  Ptr: i8* %call1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0), i32* %ar1)

; Test with printf library call using a format string that modifies the memory
; of the parameter
@.str7 = internal global [13 x i8] c"Set val-%zn\0A\00", align 1
define internal void @test07() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1
  %arindex5 = getelementptr i32, i32* %ar1, i64 4
  %ld.arindex5 = load i32, i32* %arindex5

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout
  %tmp = call i32 (i8*, ...) @printf(
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0),
    i32* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test07:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %arindex5	<->  %tmp = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)

; Test with library call to snprintf (format string is 3rd argument), passing
; pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str8 = private constant [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test08() {
entry:
  %buffer = tail call noalias i8* @malloc(i64 100)
  %ld.buffer = load i8, i8* %buffer
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test08:
; CHECK: Both ModRef:  Ptr: i8* %buffer	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i8* %call1	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str8, i64 0, i64 0), i32* %ar1)

; Test with library call to snprintf that modifies argument
@.str9 = internal global [14 x i8] c"Set val-%hhn\0A\00", align 1
define internal void @test09() {
entry:
  %buffer = tail call noalias i8* @malloc(i64 100)
  %ld.buffer = load i8, i8* %buffer
  %call1 = tail call noalias i8* @malloc(i64 256)
  %ld.call1 = load i8, i8* %call1
  %arindex = getelementptr i8, i8* %call1, i64 4
  %ld.arindex = load i8, i8* %arindex

  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100,
    i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str9, i64 0, i64 0),
    i8* %arindex)

  ret void
}
; CHECK-LABEL: Function: test09:
; CHECK: Both ModRef:  Ptr: i8* %buffer	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str9, i64 0, i64 0), i8* %arindex)
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str9, i64 0, i64 0), i8* %arindex)
; CHECK: Both ModRef:  Ptr: i8* %arindex	<->  %tmp = call i32 (i8*, i64, i8*, ...) @snprintf(i8* %buffer, i64 100, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str9, i64 0, i64 0), i8* %arindex)

; Test with call to library function, but with a pointer that escapes, which
; will require a conservative result of Mod/Ref.
@.str10 = private constant [13 x i8] c"Result = %p\0A\00", align 1
@nabout10 = external dso_local local_unnamed_addr global %struct._IO_FILE*
define internal void @test10() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._IO_FILE*, %struct._IO_FILE** @nabout10
  %fp.1 = getelementptr %struct._IO_FILE, %struct._IO_FILE* %fp, i64 0, i32 1
  store i8* %call1, i8** %fp.1

  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str10, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test10:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str10, i64 0, i64 0), i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  %tmp = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str10, i64 0, i64 0), i32* %ar1)


define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) {
  call void @test01()
  call void @test02()
  call void @test03()
  call void @test04()
  call void @test05()
  call void @test06()
  call void @test07()
  call void @test08()
  call void @test09()
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local i32 @printf(i8* nocapture readonly, ...)
declare dso_local i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...)
declare dso_local i32 @fflush(%struct._IO_FILE* nocapture)
declare dso_local i32 @snprintf(i8* nocapture, i64, i8* nocapture readonly, ...)
declare dso_local i8* @gets(i8*)
