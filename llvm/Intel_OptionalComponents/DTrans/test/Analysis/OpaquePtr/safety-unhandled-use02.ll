; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that instructions not explicitly modeled in the DTransSafetyAnalyzer
; result in the "Unhandled use" safety flag.

; This case is using an structure element pointee with an instruction that
; is not one of the supported instructions by the DTrans Safety Analyzer.
;
; This case is similar to safety-unhandled-use01.ll, but the 'trunc'
; instruction is applied to a pointer to a structure member, rather
; than the structure itself.

%struct.test01 = type { ptr, ptr }

define void @test01() {
  %var = alloca %struct.test01
  %v32 = alloca i32
  %f1 = getelementptr %struct.test01, ptr %var, i64 0, i32 1
  
  ; %pti will be tracked as aliasing a pointer that is the address of a field
  ; within a structure. The 'trunc' instruction should trigger 'Unhandled use'
  ; on the structure the field address came from.
  %pti = ptrtoint ptr %f1 to i64
  %t = trunc i64 %pti to i32
  
  store i32 %t, ptr %v32
  ret void;
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Local instance | Unhandled use{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i64 0, i32 1}  ; i64*
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64*, i64* }

!intel.dtrans.types = !{!2}

