; REQUIRES: asserts

; ptrtype-libinfo01.ll testcase verifies that PtrTypeAnalyzer does not set
; UNHANDLED on pointers used for Linux variant library functions when metadata
; is missing for the library functions. This testcase
; (ptrtype-win-libinfo03.ll) also does verify the same but for Windows variant
; library functions.

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that the DTrans PtrTypeAnalyzer does not set UNHANDLED on pointers used
; for library functions that are declared without DTrans metadata attached to
; them because an internal lookup table can be used to resolve their types.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30145"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

define void @test_libfunc() {
  %p8.1 = alloca i8
; CHECK: %p8.1 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %atexit = call i32 @atexit(ptr %p8.1)
  ret void
}

; Library functions declared without DTrans metadata.
declare i32 @atexit(ptr)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !1, !1, !1, !1, !2} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!3}

