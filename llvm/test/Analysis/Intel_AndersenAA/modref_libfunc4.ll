; RUN: opt < %s  -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)'  -aa-pipeline=anders-aa -evaluate-loopcarried-alias -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; This test is similar to modref_libfunc1.ll except printf and fprintf have
; definitions in IR. Also, IR in this test represents FILE I/O on Windows.

; This tests the ModRef analysis for various forms of printf routines.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%struct._iobuf = type { ptr }
%struct.__crt_locale_pointers = type { ptr, ptr }
%struct.__crt_locale_data = type opaque
%struct.__crt_multibyte_data = type opaque

@nabout = internal global ptr null
@__local_stdio_printf_options._OptionsStorage = internal global i64 0

define weak_odr dso_local ptr @__local_stdio_printf_options() {
  ret ptr @__local_stdio_printf_options._OptionsStorage
}

define internal void @printf(ptr %0, ...) {
  %2 = alloca ptr, align 8
  %3 = bitcast ptr %2 to ptr
  call void @llvm.lifetime.start.p0i8(i64 8, ptr nonnull %3)
  call void @llvm.va_start(ptr nonnull %3)
  %4 = load ptr, ptr %2
  %5 = call ptr @__acrt_iob_func(i32 1)
  %6 = call ptr @__local_stdio_printf_options()
  %7 = load i64, ptr %6
  call void @__stdio_common_vfprintf(i64 %7, ptr %5, ptr %0, ptr null, ptr %4)
  call void @llvm.va_end(ptr nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 8, ptr nonnull %3)
  ret void
}

define internal void @fprintf(ptr %0, ptr %1, ...) {
  %3 = alloca ptr, align 8
  %4 = bitcast ptr %3 to ptr
  call void @llvm.lifetime.start.p0i8(i64 8, ptr nonnull %4)
  call void @llvm.va_start(ptr nonnull %4)
  %5 = load ptr, ptr %3
  %6 = call ptr @__local_stdio_printf_options()
  %7 = load i64, ptr %6
  call void @__stdio_common_vfprintf(i64 %7, ptr %0, ptr %1, ptr null, ptr %5
)
  call void @llvm.va_end(ptr nonnull %4)
  call void @llvm.lifetime.end.p0i8(i64 8, ptr nonnull %4)
  ret void
}

; Test with library call to fprintf, passing pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str = private constant [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test01() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1

  %fp = load ptr, ptr @nabout
  call void (ptr, ptr, ...) @fprintf(ptr %fp,
    ptr @.str,
    ptr %ar1)

  ret void
}
; CHECK-LABEL: Function: test01:
; CHECK: Just Ref:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str, ptr %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str, ptr %ar1)

; Test with library call to fprintf, passing pointer argument, and non-constant string
@.str2 = internal global [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test02() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1

  %fp = load ptr, ptr @nabout
  call void (ptr, ptr, ...) @fprintf(ptr %fp,
    ptr @.str2,
    ptr %ar1)

  ret void
}
; CHECK-LABEL: Function: test02:
 ;CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str2, ptr %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str2, ptr %ar1)

; The with library call using a format string that is not a compile time string
@.str3 = private constant [4 x i8] c"%s\0A\00", align 1
define internal void @test03() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1
  %str = call ptr @gets(ptr getelementptr inbounds ([4 x i8], ptr @.str3, i64 0, i64 0))

  %fp = load ptr, ptr @nabout
  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr %str, ptr %ar1)

  ret void
}
; CHECK-LABEL: Function: test03:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr %str, ptr %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr %str, ptr %ar1)

; Test with library call using a format string that modifies the memory of the parameter
@.str4 = internal global [13 x i8] c"Set val--%n\0A\00", align 1
define internal void @test04() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1
  %arindex5 = getelementptr i32, ptr %ar1, i64 4
  %ld.arindex5 = load i32, ptr %arindex5

  %fp = load ptr, ptr @nabout
  call void (ptr, ptr, ...) @fprintf(ptr %fp,
    ptr @.str4,
    ptr %arindex5)

  ret void
}
; CHECK-LABEL: Function: test04:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str4, ptr %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str4, ptr %arindex5)
; CHECK:   Both ModRef:  Ptr: i32* %arindex5	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str4, ptr %arindex5)

; Test with library call using a format string that modifies the memory of the parameter and
; the %n has a length modifier.
@.str5 = internal global [13 x i8] c"Set val-%hn\0A\00", align 1
define internal void @test05() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 512)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i16, ptr %ar1
  %arindex5 = getelementptr i16, ptr %ar1, i64 4
  %ld.arindex5 = load i16, ptr %arindex5

  %fp = load ptr, ptr @nabout
  call void (ptr, ptr, ...) @fprintf(ptr %fp,
    ptr @.str5,
    ptr %arindex5)

  ret void
}
; CHECK-LABEL: Function: test05:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str5, ptr %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str5, ptr %arindex5)
; CHECK: Both ModRef:  Ptr: i16* %arindex5	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str5, ptr %arindex5)

; Test with library call to printf (format string is 1st argument), passing
; pointer argument, and constant string.
; Pointer argument should be treated as referenced only.
@.str6 = private constant [13 x i8] c"Result = %p\0A\00", align 1
define internal void @test06() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1

  call void (ptr, ...) @printf(
    ptr @.str6,
    ptr %ar1)

  ret void
}
; CHECK-LABEL: Function: test06:
; CHECK: Just Ref:  Ptr: i8* %call1	<->  call void (ptr, ...) @printf(ptr @.str6, ptr %ar1)
; CHECK: Just Ref:  Ptr: i32* %ar1	<->  call void (ptr, ...) @printf(ptr @.str6, ptr %ar1)

; Test with printf library call using a format string that modifies the memory
; of the parameter
@.str7 = internal global [13 x i8] c"Set val-%zn\0A\00", align 1
define internal void @test07() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1
  %arindex5 = getelementptr i32, ptr %ar1, i64 4
  %ld.arindex5 = load i32, ptr %arindex5

  %fp = load ptr, ptr @nabout
  call void (ptr, ...) @printf(
    ptr @.str7,
    ptr %arindex5)

  ret void
}
; CHECK-LABEL: Function: test07:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ...) @printf(ptr @.str7, ptr %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (ptr, ...) @printf(ptr @.str7, ptr %arindex5)
; CHECK: Both ModRef:  Ptr: i32* %arindex5	<->  call void (ptr, ...) @printf(ptr @.str7, ptr %arindex5)

; Test with call to library function, but with a pointer that escapes, which
; will require a conservative result of Mod/Ref.
@.str08 = private constant [13 x i8] c"Result = %p\0A\00", align 1
@nabout08 = external dso_local local_unnamed_addr global ptr
define internal void @test08() {
entry:
  %call1 = tail call noalias ptr @malloc(i64 1024)
  %ld.call1 = load i8, ptr %call1
  %ar1 = bitcast ptr %call1 to ptr
  %ld.ar1 = load i32, ptr %ar1

  %fp = load ptr, ptr @nabout08
  %fp.1 = getelementptr %struct._iobuf, ptr %fp, i64 0, i32 0
  store ptr %call1, ptr %fp.1

  call void (ptr, ptr, ...) @fprintf(ptr %fp,
    ptr @.str08,
    ptr %ar1)

  ret void
}
; CHECK-LABEL: Function: test08:
; CHECK: Both ModRef:  Ptr: i8* %call1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str08, ptr %ar1)
; CHECK: Both ModRef:  Ptr: i32* %ar1	<->  call void (ptr, ptr, ...) @fprintf(ptr %fp, ptr @.str08, ptr %ar1)


define dso_local i32 @main(i32 %argc, ptr nocapture readonly %argv) {
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

declare dso_local noalias ptr @malloc(i64)
declare dso_local i32 @fflush(ptr nocapture)
declare dso_local ptr @gets(ptr)
declare dso_local ptr @__acrt_iob_func(i32)
declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture)
declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture)
declare void @__stdio_common_vfprintf(i64, ptr, ptr, ptr, ptr)
declare void @llvm.va_start(ptr)
declare void @llvm.va_end(ptr)
