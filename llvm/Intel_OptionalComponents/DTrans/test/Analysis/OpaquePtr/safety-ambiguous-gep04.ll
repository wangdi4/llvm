; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer.

; "Ambiguous GEP" case where type is used with a single GEP index, but using
; a type that does not match the expected type.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i16, i16 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct.a) !intel.dtrans.func.type !4 {
  %pStruct.a.as.b = bitcast %struct.test01a* %pStruct.a to %struct.test01b*
  %ambig_gep = getelementptr %struct.test01b, %struct.test01b* %pStruct.a.as.b, i64 4
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; "Ambiguous GEP" case where the type is used with nested structure access, using
; the incorrect type.
%struct.test02a0 = type { i32, %struct.test02a1 }
%struct.test02a1 = type { i64, %struct.test02a2 }
%struct.test02a2 = type { i32, i32 }
%struct.test02b0 = type { i32, %struct.test02b1 }
%struct.test02b1 = type { i64, %struct.test02b2 }
%struct.test02b2 = type { i32, i16, i16 }
define void @test02(%struct.test02a0* "intel_dtrans_func_index"="1" %pStruct.a) !intel.dtrans.func.type !11 {
  %pStruct.a.as.b = bitcast %struct.test02a0* %pStruct.a to %struct.test02b0*
  %ambig_gep = getelementptr %struct.test02b0, %struct.test02b0* %pStruct.a.as.b, i64 0, i32 1, i32 1, i32 2
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a0
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a0

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a1
; CHECK: Safety data: Ambiguous GEP | Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a1

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a2
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a2

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b0
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b0

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b1
; CHECK: Safety data: Ambiguous GEP | Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b1

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b2
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b2


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02a1 zeroinitializer, i32 0}  ; %struct.test02a1
!6 = !{i64 0, i32 0}  ; i64
!7 = !{%struct.test02a2 zeroinitializer, i32 0}  ; %struct.test02a2
!8 = !{%struct.test02b1 zeroinitializer, i32 0}  ; %struct.test02b1
!9 = !{%struct.test02b2 zeroinitializer, i32 0}  ; %struct.test02b2
!10 = !{%struct.test02a0 zeroinitializer, i32 1}  ; %struct.test02a0*
!11 = distinct !{!10}
!12 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test01b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }
!14 = !{!"S", %struct.test02a0 zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test02a1 }
!15 = !{!"S", %struct.test02a1 zeroinitializer, i32 2, !6, !7} ; { i64, %struct.test02a2 }
!16 = !{!"S", %struct.test02a2 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!17 = !{!"S", %struct.test02b0 zeroinitializer, i32 2, !1, !8} ; { i32, %struct.test02b1 }
!18 = !{!"S", %struct.test02b1 zeroinitializer, i32 2, !6, !9} ; { i64, %struct.test02b2 }
!19 = !{!"S", %struct.test02b2 zeroinitializer, i32 3, !1, !2, !2} ; { i32, i16, i16 }

!intel.dtrans.types = !{!12, !13, !14, !15, !16, !17, !18, !19}
