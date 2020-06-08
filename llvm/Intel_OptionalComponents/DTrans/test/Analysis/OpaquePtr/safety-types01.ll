; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Basic test of the DTransSafetyAnalyzer pass to check that TypeInfo
; objects get created and reported for the structure types. Also,
; checks that 'Contains nested structure' and 'Nested structure'
; safety bit get set.

%struct.test01a0 = type { %struct.test01a1 }
%struct.test01a1 = type { [4 x %struct.test01a2] }
%struct.test01a2 = type { %struct.test01a2impl* }
%struct.test01a2impl = type { i32, %struct.test01a3*, %struct.test01a4* }
%struct.test01a3 = type { i16, i16 }
%struct.test01a4 = type { i64, [8 x i16] }
define void @test01() {
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a0
; CHECK: Safety data: Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a1
; CHECK: Safety data: Nested structure | Contains nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a2
; CHECK: Safety data: Nested structure

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a2impl
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a3
; CHECK: Safety data: No issues found

; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01a4
; CHECK: Safety data: No issues found

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [4 x %struct.test01a2]
; CHECK: Number of elements: 4
; CHECK: Element DTrans Type: %struct.test01a2

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [8 x i16]
; CHECK: Number of elements: 8
; CHECK: Element DTrans Type: i16


!1 = !{!"R", %struct.test01a1 zeroinitializer, i32 0}  ; %struct.test01a1
!2 = !{!"A", i32 4, !3}  ; [4 x %struct.test01a2]
!3 = !{!"R", %struct.test01a2 zeroinitializer, i32 0}  ; %struct.test01a2
!4 = !{!5, i32 1}  ; %struct.test01a2impl*
!5 = !{!"R", %struct.test01a2impl zeroinitializer, i32 0}  ; %struct.test01a2impl
!6 = !{i32 0, i32 0}  ; i32
!7 = !{!8, i32 1}  ; %struct.test01a3*
!8 = !{!"R", %struct.test01a3 zeroinitializer, i32 0}  ; %struct.test01a3
!9 = !{!10, i32 1}  ; %struct.test01a4*
!10 = !{!"R", %struct.test01a4 zeroinitializer, i32 0}  ; %struct.test01a4
!11 = !{i64 0, i32 0}  ; i64
!12 = !{i16 0, i32 0}  ; i16
!13 = !{!"A", i32 8, !12}  ; [8 x i16]
!14 = !{!"S", %struct.test01a0 zeroinitializer, i32 1, !1} ; { %struct.test01a1 }
!15 = !{!"S", %struct.test01a1 zeroinitializer, i32 1, !2} ; { [4 x %struct.test01a2] }
!16 = !{!"S", %struct.test01a2 zeroinitializer, i32 1, !4} ; { %struct.test01a2impl* }
!17 = !{!"S", %struct.test01a2impl zeroinitializer, i32 3, !6, !7, !9} ; { i32, %struct.test01a3*, %struct.test01a4* }
!18 = !{!"S", %struct.test01a3 zeroinitializer, i32 2, !12, !12} ; { i16, i16 }
!19 = !{!"S", %struct.test01a4 zeroinitializer, i32 2, !11, !13} ; { i64, [8 x i16] }

!dtrans_types = !{!14, !15, !16, !17, !18, !19}
