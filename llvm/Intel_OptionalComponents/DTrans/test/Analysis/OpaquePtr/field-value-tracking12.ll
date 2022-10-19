; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that field value sets get marked as 'incomplete' when
; an unknown value is stored to the field which is tracking
; multiple values.

%struct.test01 = type { i32 }
; Initialize the structure to add 0 to the value set.
@var01 = internal global %struct.test01 zeroinitializer
define void @test01(i32 %in1) {
  %local = alloca %struct.test01
  %f0 = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 0

  ; Store a constant to make the value set multiple value.
  store i32 1, i32* %f0

  ; Store an unknown value to make the value set incomplete.
  store i32 %in1, i32* %f0
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 0, 1 ] <incomplete>
; CHECK:   Safety data: Global instance | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!2}
