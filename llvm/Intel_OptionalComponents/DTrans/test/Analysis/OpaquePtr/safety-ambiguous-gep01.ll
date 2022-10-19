; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test detection of "Ambiguous GEP" safety condition by DTrans safety analyzer.

; The input parameter is declared as one type, but gets loaded as a
; different type, resulting in an "Ambiguous GEP" on the GEP pointer
; operand.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i16, i16, i16, i16 }
define void @test01(%struct.test01a** "intel_dtrans_func_index"="1" %ppStruct.a) !intel.dtrans.func.type !4 {
  %pStruct.a.as.ppb = bitcast %struct.test01a** %ppStruct.a to %struct.test01b**
  %pStruct.b = load %struct.test01b*, %struct.test01b** %pStruct.a.as.ppb
  %pField = getelementptr %struct.test01b, %struct.test01b* %pStruct.b, i64 0, i32 3
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; Merging pointers of two different structures together, and then using
; the result in a GEP prevents DTrans from knowing which structure type is
; being accessed.
%struct.test02a = type { i32 }
%struct.test02b = type { i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct1a, %struct.test02b* "intel_dtrans_func_index"="2" %pStruct1b) !intel.dtrans.func.type !7 {
  %pStruct1a.as.p8 = bitcast %struct.test02a* %pStruct1a to i8*
  %pStruct1b.as.p8 = bitcast %struct.test02b* %pStruct1b to i8*
  %ambig_ptr = select i1 undef, i8* %pStruct1a.as.p8, i8* %pStruct1b.as.p8
  %ambig_gep = getelementptr i8, i8* %ambig_ptr, i64 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Ambiguous GEP | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Ambiguous GEP | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02b


; Mixing structure pointer and array with an alternative representation of the
; first field is not supported, and should be marked as "Ambiguous GEP"
%struct.test03a = type { %struct.test03b, i32 }
%struct.test03b = type { [8 x i64] }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct.a) !intel.dtrans.func.type !12 {
  %pStruct.a.as.ar = bitcast %struct.test03a* %pStruct.a to [64 x i8]*
  %first = getelementptr inbounds [64 x i8], [64 x i8]* %pStruct.a.as.ar, i64 0, i64 0
  %value = load i8, i8* %first
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Ambiguous GEP | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Ambiguous GEP | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = distinct !{!3}
!5 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!6 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!7 = distinct !{!5, !6}
!8 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!9 = !{!"A", i32 8, !10}  ; [8 x i64]
!10 = !{i64 0, i32 0}  ; i64
!11 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test01b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i16, i16, i16, i16 }
!15 = !{!"S", %struct.test02a zeroinitializer, i32 1, !1} ; { i32 }
!16 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i32 }
!17 = !{!"S", %struct.test03a zeroinitializer, i32 2, !8, !1} ; { %struct.test03b, i32 }
!18 = !{!"S", %struct.test03b zeroinitializer, i32 1, !9} ; { [8 x i64] }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
