; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing the address of a field to a function to trigger the "Field
; address taken call" safety bit.

%struct.test01 = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01
  %pFieldAddr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  call void @test01h(i32* %pFieldAddr)
  ret void
}
define void @test01h(i32* "intel_dtrans_func_index"="1" %pAddr) !intel.dtrans.func.type !3 {
  store i32 0, i32* %pAddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Local instance | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!4}
