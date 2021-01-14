; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that calls that have pointers to aggregate types passed and returned do
; not cause safety flags to be set when they are used as the expected types.

%struct.test01 = type { i32, i32 }
define %struct.test01* @test01(%struct.test01* %pStruct, i64 %idx) !dtrans_type !2 {
  %res = getelementptr %struct.test01, %struct.test01* %pStruct, i64 %idx
  ret %struct.test01* %res
}

define void @test01c() {
  %mem = call i8* @malloc(i64 40)
  %head = bitcast i8* %mem to %struct.test01*
  %elem = call %struct.test01* @test01(%struct.test01* %head, i64 3)
  %fieldAddr = getelementptr %struct.test01, %struct.test01* %head, i64 0, i32 1
  store i32 0, i32* %fieldAddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


declare i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !3, !5}  ; %struct.test01* (%struct.test01*, i64)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}
