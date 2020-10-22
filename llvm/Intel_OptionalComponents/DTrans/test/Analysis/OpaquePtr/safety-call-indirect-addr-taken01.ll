; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-usecrulecompat -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -dtrans-usecrulecompat -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test for indirect function calls which need to have the structure type passed
; marked as "Address taken" because there is no compatible type that could be
; operated upon by an address taken function.

%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }

@myarg = internal global %struct.test01a { i32 3, i32 5 }
@fp = internal global i32 (%struct.test01a*)* null, !dtrans_type !5

define i32 @target1(%struct.test01a* %pStruct) !dtrans_type !2 {
  %fieldAddr = getelementptr inbounds %struct.test01a, %struct.test01a* %pStruct, i32 0, i32 0
  %res = load i32, i32* %fieldAddr
  ret i32 %res
}

define i32 @target2(%struct.test01b* %pStruct) !dtrans_type !6 {
  %fieldAddr = getelementptr inbounds %struct.test01b, %struct.test01b* %pStruct, i32 0, i32 1
  %res = load i32, i32* %fieldAddr
  ret i32 %res
}

define i32 @main() {
  %fptr = load i32 (%struct.test01a*)*, i32 (%struct.test01a*)** @fp, align 8
  %res = call i32 %fptr(%struct.test01a* @myarg), !dtrans_type !2
  ret i32 %res
}
; This case differs from safety-call-indirect-safe01.ll, in that it will
; continue to have the "Address taken" bit set on the passed structure type,
; after support for analyzing indirect function calls is implemented, even
; when using the -dtrans-usecrulecompat option.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Global instance | Has initializer list | Address taken{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (%struct.test01a*)
!3 = !{!4, i32 1}  ; %struct.test01a*
!4 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!5 = !{!2, i32 1}  ; i32 (%struct.test01a*)*
!6 = !{!"F", i1 false, i32 1, !1, !7}  ; i32 (%struct.test01b*)
!7 = !{!8, i32 1}  ; %struct.test01b*
!8 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!9 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!9, !10}
