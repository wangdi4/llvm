; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test validates correct handling of pointer alignment checking idioms
; when performing the safety check.

; Check for 8-byte alignment
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %p) !dtrans_type !2 {
  %t1 = ptrtoint %struct.test01* %p to i64
  %t2 = and i64 %t1, 7
  %cmp = icmp eq i64 %t2, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


; Check for 8-byte alignment with an extraneous bit set
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %p) !dtrans_type !6 {
  %t1 = ptrtoint %struct.test02* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %cmp = icmp eq i64 %t3, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: No issues found


; Check for variable bitmask -- this may be OK, but we don't need it so it's
; easier to exclude it and not worry about unintended consequences.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %p, i64 %mask) !dtrans_type !9 {
  %t1 = ptrtoint %struct.test03* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, %mask
  %cmp = icmp eq i64 %t3, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Unhandled use{{ *}}


; Check for comparison of two masked pointers
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %p1, %struct.test04* %p2) !dtrans_type !13 {
  %t1 = ptrtoint %struct.test04* %p1 to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %t4 = ptrtoint %struct.test04* %p2 to i64
  %t5 = or i64 %t4, 8
  %t6 = and i64 %t5, 7
  %cmp = icmp eq i64 %t3, %t6
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Unhandled use{{ *}}


; Check for alignment check on a field element address
%struct.test05 = type <{ i16, i32 }>
define void @test05(%struct.test05* %p) !dtrans_type !17 {
  %t0 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 1
  %t1 = ptrtoint i32* %t0 to i64
  %t2 = and i64 %t1, 7
  %cmp = icmp eq i64 %t2, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Unhandled use{{ *}}


; Alignment checks on pointer-to-pointer types for cases that are not supported
; on pointer-types should not cause an issue, but we don't have cases that
; require it currently, so mark it as unhandled.
%struct.test06 = type { i32, i32 }
define void @test06(%struct.test06** %p1, %struct.test06** %p2) !dtrans_type !20 {
  %t1 = ptrtoint %struct.test06** %p1 to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %t4 = ptrtoint %struct.test06** %p2 to i64
  %t5 = or i64 %t4, 8
  %t6 = and i64 %t5, 7
  %cmp = icmp eq i64 %t3, %t6
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: Unhandled use{{ *}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 2, !3, !10, !12}  ; void (%struct.test03*, i64)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{i64 0, i32 0}  ; i64
!13 = !{!"F", i1 false, i32 2, !3, !14, !14}  ; void (%struct.test04*, %struct.test04*)
!14 = !{!15, i32 1}  ; %struct.test04*
!15 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!16 = !{i16 0, i32 0}  ; i16
!17 = !{!"F", i1 false, i32 1, !3, !18}  ; void (%struct.test05*)
!18 = !{!19, i32 1}  ; %struct.test05*
!19 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!20 = !{!"F", i1 false, i32 2, !3, !21, !21}  ; void (%struct.test06**, %struct.test06**)
!21 = !{!22, i32 2}  ; %struct.test06**
!22 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!23 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!24 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!25 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!26 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!27 = !{!"S", %struct.test05 zeroinitializer, i32 2, !16, !1} ; <{ i16, i32 }>
!28 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!23, !24, !25, !26, !27, !28}
