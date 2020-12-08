; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to an aggregate type as an i8* type that is safe for
; DTrans.

; Test with using the i8* parameter as a type that is compatible with the use in
; the caller.
define void @test01(%struct.test01** %pStruct) !dtrans_type !1 {
  %ps_addr0 = getelementptr %struct.test01*, %struct.test01** %pStruct, i64 0
  %ps_addr1 = getelementptr %struct.test01*, %struct.test01** %pStruct, i64 1
  %ps0 = load %struct.test01*, %struct.test01** %ps_addr0
  %ps1 = load %struct.test01*, %struct.test01** %ps_addr1
  %less = call i1 bitcast (i1 (i8*, i8*)* @test01less to
               i1 (%struct.test01*, %struct.test01*)*)(%struct.test01* %ps0, %struct.test01* %ps1)
  ret void
}

%struct.test01 = type { i32, i32 }
define i1 @test01less(i8* %p0, i8* %p1) !dtrans_type !6 {
  %ps0 = bitcast i8* %p0 to %struct.test01*
  %ps1 = bitcast i8* %p1 to %struct.test01*
  %fs0 = getelementptr %struct.test01, %struct.test01* %ps0, i64 0, i32 0
  %fs1 = getelementptr %struct.test01, %struct.test01* %ps1, i64 0, i32 0
  %v0 = load i32, i32* %fs0
  %v1 = load i32, i32* %fs1
  %cmp = icmp slt i32 %v0, %v1
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


!1 = !{!"F", i1 false, i32 1, !2, !3}  ; void (%struct.test01**)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 2}  ; %struct.test01**
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i32 0, i32 0}  ; i32
!6 = !{!"F", i1 false, i32 2, !7, !8, !8}  ; i1 (i8*, i8*)
!7 = !{i1 0, i32 0}  ; i1
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !5, !5} ; { i32, i32 }

!dtrans_types = !{!9}
