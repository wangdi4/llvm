; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to an aggregate type to a function that expects a
; pointer to a different aggregate type. This should trigger the "Bad casting"
; safety flag.

%struct.test01a = type { i64 }
%struct.test01b = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01a
  %pStruct.as.pb = bitcast %struct.test01a* %pStruct to %struct.test01b*
  call void @test01callee(%struct.test01b* %pStruct.as.pb)
  ret void
}

define void @test01callee(%struct.test01b* %pStruct) !dtrans_type !3 {
  %fieldAddr = getelementptr %struct.test01b, %struct.test01b* %pStruct, i64 0, i32 1
  store i32 0, i32* %fieldAddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Local instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01b*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01b*
!6 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!7 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i64 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!dtrans_types = !{!7, !8}
