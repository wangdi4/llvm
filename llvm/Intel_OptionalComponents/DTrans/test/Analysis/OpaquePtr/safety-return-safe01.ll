; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases that are safe uses of the return instruction.

; Test with return of pointer of the expected type
; Return a properly typed pointer to a structure.
%struct.test01 = type { i32, i32 }
define %struct.test01* @test1(%struct.test01* %pStruct) !dtrans_type !2 {
  ret %struct.test01* %pStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found


; Test with return of null pointer.
%struct.test02 = type { i32, i32 }
define %struct.test02* @test2() !dtrans_type !5 {
  ret %struct.test02* null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found


; Return an i64 type that does not represent a pointer to a
; structure.
%struct.test03 = type { i64 }
define i64 @test3(%struct.test03* %pStruct) !dtrans_type !9 {
  %addr = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 0
  %val = load i64, i64* %addr
  ret i64 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !3}  ; %struct.test01* (%struct.test01*)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"F", i1 false, i32 0, !6}  ; %struct.test02* ()
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{i64 0, i32 0}  ; i64
!9 = !{!"F", i1 false, i32 1, !8, !10}  ; i64 (%struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 1, !8} ; { i64 }

!dtrans_types = !{!12, !13, !14}
