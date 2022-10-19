; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing into a pointer field type using a scalar type.

%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i8 %value) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast i32** %pField to i8*
  store i8 %value, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct, i16 %value) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast i32** %pField to i16*
  store i16 %value, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


; This case triggers a "Mismatched element access" even though it is using
; a pointer sized int to store a pointer because there is no information about
; what the integer may actually represent.
%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct, i64 %value) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32** %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


; This case does not trigger a "Mismatched element access" because it is using a
; pointer sized int to store a value that is known to be a pointer of the correct
; type.
%struct.test04 = type { i32*, i32*, i32* }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStruct, i32* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test04, %struct.test04* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32** %pField to i64*
  %pValue.as.i64 = ptrtoint i32* %value to i64
  store i64 %pValue.as.i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test04


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8, !1}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!13 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }

!intel.dtrans.types = !{!10, !11, !12, !13}
