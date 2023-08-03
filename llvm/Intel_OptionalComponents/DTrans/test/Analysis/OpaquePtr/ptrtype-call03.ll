; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery for "call" instructions


; Special case of a call that is an inline-asm which returns a pointer.
; Currently, the result will be flagged as UNHANDLED. We may need metadata from
; the front-end if we want to handle it, but the benchmarks that contain
; inline-asm code do not need DTrans transformations.
@G = private unnamed_addr constant [1 x i8] c"\00", align 1
define internal void @test01() {
  %local = getelementptr [1 x i8], ptr @G, i64 0, i64 0
  call ptr asm "nop", "=r,r"(ptr %local)
  ret void
}
; CHECK-LABEL: internal void @test01
; CHECK:  %1 = call ptr asm "nop", "=r,r"(ptr %local)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNHANDLED


; Special case with user being an inline-asm instruction.
; Currently, the parameter will be flagged as UNHANDLED. We may need
; metadata from the front-end if we want to handle it, but the benchmarks
; that contain inline-asm code do not need DTrans transformations.
define internal void @test02(ptr %buf) {
  %bc = bitcast ptr %buf to ptr
  %tmp = call { i64, ptr } asm sideeffect "cld; rep; stosq", "={cx},={di},{ax},0,1,~{memory},~{dirflag},~{fpsr},~{flags}"(i32 0, i64 16, i64* %bc)
  ret void
}
; CHECK-LABEL:  Input Parameters: test02
; CHECK:    Arg 0: ptr %buf
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNHANDLED

!intel.dtrans.types = !{}
