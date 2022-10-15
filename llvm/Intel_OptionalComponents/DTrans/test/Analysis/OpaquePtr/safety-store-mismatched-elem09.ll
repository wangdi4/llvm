; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing an incorrect scalar type to a location that holds the
; address of a nested structure member.


%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct, i8 %value) !intel.dtrans.func.type !4 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField.as.p8 = bitcast %struct.test01b* %pField to i8*
  store i8 %value, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; Safety data: Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


 %struct.test02a = type { %struct.test02b }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct, i16 %value) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField.as.p16 = bitcast %struct.test02b* %pField to i16*
  store i16 %value, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


%struct.test03a = type { %struct.test03b }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct, i64 %value) !intel.dtrans.func.type !10 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test03b* %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b


; This access could be considered a whole structure reference, but we do not
; handle this case in DTrans for simplicity, so it should be marked as
; "mismatched element access".
%struct.test04a = type { %struct.test04b }
%struct.test04b = type { i32, i32 }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct, i64 %value) !intel.dtrans.func.type !13 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test04b* %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04b


; Access of 'i32' field within the nested structure as a different
; structure type.
%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
%struct.test05c = type { i64 }
define void @test05(%struct.test05a* "intel_dtrans_func_index"="1" %pStruct, %struct.test05c* "intel_dtrans_func_index"="2" %pStruct5c) !intel.dtrans.func.type !18 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.ppS5c = bitcast %struct.test05b* %pField to %struct.test05c**
  store %struct.test05c* %pStruct5c, %struct.test05c** %pField.as.ppS5c
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05c
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test05c

; A safe access to a nested element.
%struct.test06a = type { %struct.test06b }
%struct.test06b = type { i32, i32, i32 }
define void @test06(%struct.test06a* "intel_dtrans_func_index"="1" %pStruct, i32 %value) !intel.dtrans.func.type !21 {
  %pField = getelementptr %struct.test06a, %struct.test06a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test06b* %pField to i32*
  store i32 %value, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test06a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06b
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test06b


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = distinct !{!6}
!8 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = distinct !{!9}
!11 = !{%struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!12 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!13 = distinct !{!12}
!14 = !{%struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!15 = !{i64 0, i32 0}  ; i64
!16 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!17 = !{%struct.test05c zeroinitializer, i32 1}  ; %struct.test05c*
!18 = distinct !{!16, !17}
!19 = !{%struct.test06b zeroinitializer, i32 0}  ; %struct.test06b
!20 = !{%struct.test06a zeroinitializer, i32 1}  ; %struct.test06a*
!21 = distinct !{!20}
!22 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!23 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!24 = !{!"S",  %struct.test02a zeroinitializer, i32 1, !5} ; { %struct.test02b }
!25 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test03a zeroinitializer, i32 1, !8} ; { %struct.test03b }
!27 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!28 = !{!"S", %struct.test04a zeroinitializer, i32 1, !11} ; { %struct.test04b }
!29 = !{!"S", %struct.test04b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!30 = !{!"S", %struct.test05a zeroinitializer, i32 1, !14} ; { %struct.test05b }
!31 = !{!"S", %struct.test05b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!32 = !{!"S", %struct.test05c zeroinitializer, i32 1, !15} ; { i64 }
!33 = !{!"S", %struct.test06a zeroinitializer, i32 1, !19} ; { %struct.test06b }
!34 = !{!"S", %struct.test06b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!intel.dtrans.types = !{!22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34}
