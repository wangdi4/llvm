; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK_OOB_F
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK_OOB_F
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK_OOB_T
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK_OOB_T

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify field value collection of structure fields results in
; the 'incomplete' value list condition for all fields when there is a 'field
; address taken' condition and -dtrans-outofboundsok=true is enabled. When
; -dtrans-outofboundsok=false, only fields directly address taken get marked as
; 'incomplete'

%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca %struct.test01
  %pA = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 0
  %pB = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 1
  store i32 1, i32* %pA
  store i32 2, i32* %pB

  ; Introduce a 'field address taken' safety condition.
  call void @unknownFunc(i32* %pB)
  ret void
}
declare !intel.dtrans.func.type !3 void @unknownFunc(i32* "intel_dtrans_func_index"="1")

; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK_OOB_F:    Single Value: i32 1
; CHECK_OOB_T:    Multiple Value: [ 1 ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 2 ] <incomplete>
; CHECK:  Safety data: Local instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!4}
