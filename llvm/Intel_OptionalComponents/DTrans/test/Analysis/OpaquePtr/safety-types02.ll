; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Basic test of the DTransSafetyAnalyzer pass collection of
; structure properties.

%struct.base = type { i32 (...)** }
%struct.derived1 = type { %struct.base, i8 }
%struct.derived2 = type { %struct.derived1, i32 }
%struct.empty = type { }
%struct.fptrs = type { void (i32, i64*, i64)*, i32, i32 }
%struct.zeroararry = type { i32, [0 x i32] }
define void @test01() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.base
; CHECK: Safety data: Nested structure | Has vtable

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.derived1
; CHECK: Safety data: Nested structure | Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.derived2
; CHECK: Safety data: Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.empty
; CHECK: Safety data: No fields in structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.fptrs
; CHECK: Safety data: Has function ptr

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.zeroararry
; CHECK: Safety data: Has zero-sized array


!1 = !{!"F", i1 true, i32 0, !2}  ; i32 (...)
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 2}  ; i32 (...)**
!4 = !{!"R", %struct.base zeroinitializer, i32 0}  ; %struct.base
!5 = !{i8 0, i32 0}  ; i8
!6 = !{!"R", %struct.derived1 zeroinitializer, i32 0}  ; %struct.derived1
!7 = !{!"F", i1 false, i32 3, !8, !2, !9, !10}  ; void (i32, i64*, i64)
!8 = !{!"void", i32 0}  ; void
!9 = !{i64 0, i32 1}  ; i64*
!10 = !{i64 0, i32 0}  ; i64
!11 = !{!7, i32 1}  ; void (i32, i64*, i64)*
!12 = !{!"A", i32 0, !2}  ; [0 x i32]
!13 = !{!"S", %struct.base zeroinitializer, i32 1, !3} ; { i32 (...)** }
!14 = !{!"S", %struct.derived1 zeroinitializer, i32 2, !4, !5} ; { %struct.base, i8 }
!15 = !{!"S", %struct.derived2 zeroinitializer, i32 2, !6, !2} ; { %struct.derived1, i32 }
!16 = !{!"S", %struct.empty zeroinitializer, i32 0} ; { }
!17 = !{!"S", %struct.fptrs zeroinitializer, i32 3, !11, !2, !2} ; { void (i32, i64*, i64)*, i32, i32 }
!18 = !{!"S", %struct.zeroararry zeroinitializer, i32 2, !2, !12} ; { i32, [0 x i32] }

!dtrans_types = !{!13, !14, !15, !16, !17, !18}