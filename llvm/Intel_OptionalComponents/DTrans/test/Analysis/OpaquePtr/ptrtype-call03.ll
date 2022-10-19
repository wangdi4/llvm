; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery for "call" instructions

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Special case of a call that is an inline-asm which returns a pointer.
; Currently, the result will be flagged as UNHANDLED. We may need metadata from
; the front-end if we want to handle it, but the benchmarks that contain
; inline-asm code do not need DTrans transformations.
@G = private unnamed_addr constant [1 x i8] c"\00", align 1
define internal void @test01() {
  %local = getelementptr [1 x i8], [1 x i8]* @G, i64 0, i64 0
  call i8* asm "nop", "=r,r"(i8* %local)
  ret void
}
; CHECK-LABEL: internal void @test01
; CHECK-NONOPAQUE:  %1 = call i8* asm "nop", "=r,r"(i8* %local)
; CHECK-OPAQUE:  %1 = call ptr asm "nop", "=r,r"(ptr %local)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNHANDLED


; Special case with user being an inline-asm instruction.
; Currently, the parameter will be flagged as UNHANDLED. We may need
; metadata from the front-end if we want to handle it, but the benchmarks
; that contain inline-asm code do not need DTrans transformations.
define internal void @test02(i8* %buf) {
  %bc = bitcast i8* %buf to i64*
  %tmp = call { i64, i64* } asm sideeffect "cld; rep; stosq", "={cx},={di},{ax},0,1,~{memory},~{dirflag},~{fpsr},~{flags}"(i32 0, i64 16, i64* %bc)
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK-NONOPAQUE:    Arg 0: i8* %buf
; CHECK-OPAQUE:    Arg 0: ptr %buf
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNHANDLED

!intel.dtrans.types = !{}
