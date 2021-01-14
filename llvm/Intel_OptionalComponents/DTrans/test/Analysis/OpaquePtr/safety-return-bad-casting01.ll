; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test return of pointer which does not match the aggregate type pointer.

; Return a pointer to a structure as a pointer to an integer
%struct.test01 = type { i64, i64 }
define i32* @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pStruct.as.p32 = bitcast %struct.test01* %pStruct to i32*
  ret i32* %pStruct.as.p32
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting{{ *$}}


; Return a pointer to a structure as a pointer to a different type of structure
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define %struct.test02b* @test02(%struct.test02a* %pStruct) !dtrans_type !7 {
  %pStruct.as.2b = bitcast %struct.test02a* %pStruct to %struct.test02b*
  ret %struct.test02b* %pStruct.as.2b
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; i32* (%struct.test01*)
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i32 0, i32 0}  ; i32
!7 = !{!"F", i1 false, i32 1, !8, !10}  ; %struct.test02b* (%struct.test02a*)
!8 = !{!9, i32 1}  ; %struct.test02b*
!9 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!10 = !{!11, i32 1}  ; %struct.test02a*
!11 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!12 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!13 = !{!"S", %struct.test02a zeroinitializer, i32 2, !6, !6} ; { i32, i32 }
!14 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i64 }

!dtrans_types = !{!12, !13, !14}
