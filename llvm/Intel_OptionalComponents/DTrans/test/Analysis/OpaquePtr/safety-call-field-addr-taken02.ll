; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing the address of a field to a function to trigger the "Field
; address taken call" safety bit.

%struct.test01 = type { [200 x i8], [200 x i8], i64, i64 }
define void @test01() {
  %pStruct = alloca %struct.test01
  %stringAddr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0, i32 0
  call void @test01h(i8* %stringAddr)
  ret void
}
define void @test01h(i8* "intel_dtrans_func_index"="1" %pAddr) !intel.dtrans.func.type !5 {
  store i8 0, i8* %pAddr
  ret void
}
; Note: DTrans sets "Mismatched argument use" on this because the
; PtrTypeAnalyzer function analyzeGEPAsBitcastEquivalent function will treat any
; GEP with all zero indices that resolves to be an i8* type as an alias to the
; object being indexed itself because the address is the same. In the
; LocalPointerAnalyzer implementation when a pointer such as this is passed to a
; defined function this triggers the "Mismatched arg use" safety flag. However,
; if the same address is passed to an external function taking an i8* argument,
; then it assumed to not be a mismatch. For now, we will attempt to replicate
; this behavior, but this could be relaxed in the future.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local instance | Mismatched argument use | Field address taken call{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{!"A", i32 200, !2}  ; [200 x i8]
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 4, !1, !1, !3, !3} ; { [200 x i8], [200 x i8], i64, i64 }

!intel.dtrans.types = !{!6}
