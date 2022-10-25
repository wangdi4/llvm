; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test validates correct handling of pointer alignment checking idioms
; when performing the safety check.

; Check for 8-byte alignment
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !3 {
  %t1 = ptrtoint %struct.test01* %p to i64
  %t2 = and i64 %t1, 7
  %cmp = icmp eq i64 %t2, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Check for 8-byte alignment with an extraneous bit set
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !5 {
  %t1 = ptrtoint %struct.test02* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %cmp = icmp eq i64 %t3, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Check for variable bitmask -- this may be OK, but we don't need it so it's
; easier to exclude it and not worry about unintended consequences.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %p, i64 %mask) !intel.dtrans.func.type !7 {
  %t1 = ptrtoint %struct.test03* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, %mask
  %cmp = icmp eq i64 %t3, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test03


; Check for comparison of two masked pointers
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %p1, %struct.test04* "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !9 {
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
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test04


; Check for alignment check on a field element address
%struct.test05 = type <{ i16, i32 }>
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !12 {
  %t0 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 1
  %t1 = ptrtoint i32* %t0 to i64
  %t2 = and i64 %t1, 7
  %cmp = icmp eq i64 %t2, 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test05


; Alignment checks on pointer-to-pointer types for cases that are not supported
; on pointer-types should not cause an issue, but we don't have cases that
; require it currently, so mark it as unhandled.
%struct.test06 = type { i32, i32 }
define void @test06(%struct.test06** "intel_dtrans_func_index"="1" %p1, %struct.test06** "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !14 {
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
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test06


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8, !8}
!10 = !{i16 0, i32 0}  ; i16
!11 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!12 = distinct !{!11}
!13 = !{%struct.test06 zeroinitializer, i32 2}  ; %struct.test06**
!14 = distinct !{!13, !13}
!15 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!17 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!18 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.test05 zeroinitializer, i32 2, !10, !1} ; <{ i16, i32 }>
!20 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20}
