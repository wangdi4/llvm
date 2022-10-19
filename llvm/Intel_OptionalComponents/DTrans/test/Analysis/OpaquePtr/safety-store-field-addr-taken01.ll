; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F

target triple = "x86_64-unknown-linux-gnu"


; Test that storing the address of a structure field results in the "Field
; address taken memory" safety bit.

%struct.test01a = type { i64, %struct.test01b* }
%struct.test01b = type { i32, i32 }
@gTest01b = internal global %struct.test01b** zeroinitializer, !intel_dtrans_type !4
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  store %struct.test01b** %pField, %struct.test01b*** @gTest01b
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01a
; CHECK_ALWAYS: Field address taken memory{{ *$}}

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01b
; CHECK_OOB_T: Field address taken memory | Global pointer{{ *$}}
; CHECK_OOB_F: Global pointer{{ *$}}


; This case is to verify that storing the address of field into another field
; (ie. the pointer location is an element pointee) results in the "Field
; address taken" safety bit.
%struct.test02a = type { i64*, i64, %struct.test02b* }
%struct.test02b = type { i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pField0 = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField1 = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  store i64* %pField1, i64** %pField0
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test02a
; CHECK_ALWAYS: Field address taken memory{{ *$}}

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test02b
; CHECK_OOB_T: Field address taken memory{{ *$}}
; CHECK_OOB_F: No issues found


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test01b zeroinitializer, i32 2}  ; %struct.test01b**
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{i64 0, i32 1}  ; i64*
!8 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!9 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!10 = distinct !{!9}
!11 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01b* }
!12 = !{!"S", %struct.test01b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }
!13 = !{!"S", %struct.test02a zeroinitializer, i32 3, !7, !1, !8} ; { i64*, i64, %struct.test02b* }
!14 = !{!"S", %struct.test02b zeroinitializer, i32 2, !3, !3} ; { i32, i32 }

!intel.dtrans.types = !{!11, !12, !13, !14}
