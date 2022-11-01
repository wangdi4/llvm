; RUN: opt < %s  -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -evaluate-loopcarried-alias -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This test is similar to modref_libfunc1.ll except printf and fprintf have
; definitions in IR. Also, IR in this test represents FILE I/O on Windows.

; This tests the ModRef analysis for various forms of printf routines.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%struct._iobuf = type { i8* }
%struct.__crt_locale_pointers = type { %struct.__crt_locale_data*, %struct.__crt_multibyte_data* }
%struct.__crt_locale_data = type opaque
%struct.__crt_multibyte_data = type opaque

@nabout = internal global %struct._iobuf* null
@__local_stdio_printf_options._OptionsStorage = internal global i64 0

define weak_odr dso_local i64* @__local_stdio_printf_options() {
  ret i64* @__local_stdio_printf_options._OptionsStorage
}

define internal void @printf(i8* %0, ...) {
  %2 = alloca i8*, align 8
  %3 = bitcast i8** %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %3)
  call void @llvm.va_start(i8* nonnull %3)
  %4 = load i8*, i8** %2
  %5 = call %struct._iobuf* @__acrt_iob_func(i32 1)
  %6 = call i64* @__local_stdio_printf_options()
  %7 = load i64, i64* %6
  call void @__stdio_common_vfprintf(i64 %7, %struct._iobuf* %5, i8* %0, %struct.__crt_locale_pointers* null, i8* %4)
  call void @llvm.va_end(i8* nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %3)
  ret void
}

define internal void @fprintf(%struct._iobuf* %0, i8* %1, ...) {
  %3 = alloca i8*, align 8
  %4 = bitcast i8** %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4)
  call void @llvm.va_start(i8* nonnull %4)
  %5 = load i8*, i8** %3
  %6 = call i64* @__local_stdio_printf_options()
  %7 = load i64, i64* %6
  call void @__stdio_common_vfprintf(i64 %7, %struct._iobuf* %0, i8* %1, %struct.__crt_locale_pointers* null, i8* %5
)
  call void @llvm.va_end(i8* nonnull %4)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4)
  ret void
}

; Test with library call to fprintf, passing pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str = private constant [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test01() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test01:
; CHECK: Just Ref:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i32* %ar1)

; Test with library call to fprintf, passing pointer argument, and non-constant string
@.str2 = internal global [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test02() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test02:
 ;CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0), i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str2, i64 0, i64 0), i32* %ar1)

; The with library call using a format string that is not a compile time string
@.str3 = private constant [4 x i8] c"%s\0A\00", align 1
define internal void @test03() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1
  %str = call i8* @gets(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str3, i64 0, i64 0))

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* %str, i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test03:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* %str, i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* %str, i32* %ar1)

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

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0),
    i32* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test04:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)
; CHECK:   Both ModRef:  Ptr: i32* %arindex5	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str4, i64 0, i64 0), i32* %arindex5)

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

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0),
    i16* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test05:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %arindex5	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str5, i64 0, i64 0), i16* %arindex5)

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

  call void (i8*, ...) @printf(
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test06:
; CHECK: Just Ref:  Ptr: i8* %call1	<->  call void (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0), i32* %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  call void (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str6, i64 0, i64 0), i32* %ar1)

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

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout
  call void (i8*, ...) @printf(
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0),
    i32* %arindex5)

  ret void
}
; CHECK-LABEL: Function: test07:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %arindex5	<->  call void (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str7, i64 0, i64 0), i32* %arindex5)

; Test with call to library function, but with a pointer that escapes, which
; will require a conservative result of Mod/Ref.
@.str08 = private constant [13 x i8] c"Result = %p\0A\00", align 1
@nabout08 = external dso_local local_unnamed_addr global %struct._iobuf*
define internal void @test08() {
entry:
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to i32*
  %ld.ar1 = load i32, i32* %ar1

  %fp = load %struct._iobuf*, %struct._iobuf** @nabout08
  %fp.1 = getelementptr %struct._iobuf, %struct._iobuf* %fp, i64 0, i32 0
  store i8* %call1, i8** %fp.1

  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp,
    i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str08, i64 0, i64 0),
    i32* %ar1)

  ret void
}
; CHECK-LABEL: Function: test08:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str08, i64 0, i64 0), i32* %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (%struct._iobuf*, i8*, ...) @fprintf(%struct._iobuf* %fp, i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str08, i64 0, i64 0), i32* %ar1)


define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) {
  call void @test01()
  call void @test02()
  call void @test03()
  call void @test04()
  call void @test05()
  call void @test06()
  call void @test07()
  call void @test08()
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local i32 @fflush(%struct._iobuf* nocapture)
declare dso_local i8* @gets(i8*)
declare dso_local %struct._iobuf* @__acrt_iob_func(i32)
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
declare void @__stdio_common_vfprintf(i64, %struct._iobuf*, i8*, %struct.__crt_locale_pointers*, i8*)
declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)
