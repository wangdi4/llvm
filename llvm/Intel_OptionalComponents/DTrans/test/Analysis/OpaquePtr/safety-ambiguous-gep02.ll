; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer.

; Input structure is bitcast, resulting in multiple type aliases for the GEP
; pointer operand. Because the type aliases do not correspond to an element
; zero access, this results in the structures being marked with "Ambiguous GEP"
%struct.test01a = type { %struct.test01b, i64 }
%struct.test01b = type { i32, i16, i16 }
%struct.test01c = type { i64 }
define internal void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct1a) !intel.dtrans.func.type !6 {
  %pStruct1a.as.1c = bitcast %struct.test01a* %pStruct1a to %struct.test01c*
  %pField = getelementptr %struct.test01c, %struct.test01c* %pStruct1a.as.1c, i64 0, i32 0
  %fieldVal = load i64, i64* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01c


; Like test01, the input pointer will have multiple types associated with it.
; However, in this case the types correspond to an element 0 access pattern, so
; should not be marked as "Ambiguous GEP"
%struct.test02a = type { %struct.test02b, i64 }
%struct.test02b = type { i32, i16 }
define internal void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct2a) !intel.dtrans.func.type !9 {
  %pStruct2a.as.2b = bitcast %struct.test02a* %pStruct2a to %struct.test02b*
  %pField = getelementptr %struct.test02b, %struct.test02b* %pStruct2a.as.2b, i64 0, i32 0
  %fieldVal = load i32, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 0}  ; i32
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!8 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { %struct.test01b, i64 }
!11 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !4, !4} ; { i32, i16, i16 }
!12 = !{!"S", %struct.test01c zeroinitializer, i32 1, !2} ; { i64 }
!13 = !{!"S", %struct.test02a zeroinitializer, i32 2, !7, !2} ; { %struct.test02b, i64 }
!14 = !{!"S", %struct.test02b zeroinitializer, i32 2, !3, !4} ; { i32, i16 }

!intel.dtrans.types = !{!10, !11, !12, !13, !14}
