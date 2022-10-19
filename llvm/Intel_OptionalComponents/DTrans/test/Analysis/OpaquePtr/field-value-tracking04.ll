; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify field value collection of structure fields for
; the "select between constant values to store" idiom.

%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca %struct.test01
  %pA = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 0
  %pB = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 1

  store i32 1, i32* %pA
  store i32 2, i32* %pB

  %chosen = select i1 undef, i32 3, i32 4
  store i32 %chosen, i32* %pB

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Single Value: i32 1
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 2, 3, 4 ] <complete>
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2}
