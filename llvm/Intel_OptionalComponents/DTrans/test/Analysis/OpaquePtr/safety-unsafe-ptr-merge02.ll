; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Unsafe pointer merge" safety condition when using PHI nodes


; Merging pointers to different types of structures together leads to
; "Unsafe pointer merge"
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
%struct.test01c = type { i32, i32 }
define void @test01(%struct.test01a* %a_in, %struct.test01b* %b_in, %struct.test01c* %c_in) !dtrans_type !2 {
entry:
  %tmpA = ptrtoint %struct.test01a* %a_in to i64
  %tmpB = ptrtoint %struct.test01b* %b_in to i64
  %tmpC = ptrtoint %struct.test01c* %c_in to i64
  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i64 [%a, %merge_AorC], [%b, %merge_BorC], [%tmpC, %block_BorC]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i64 [%d, %merge], [%c, %block_AorB], [%tmpA, %entry]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i64 [%d, %merge], [%c, %block_AorB], [%tmpB, %block_BorC]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i64 [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
  br i1 undef, label %block_A, label %block_B

exit_A:
  %badA = inttoptr i64 %a to %struct.test01a*
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: Unsafe pointer merge{{ *$}}


; This case is merging pointers to pointers of the aggregate types. This is
; also marked as unsafe because when the pointers are dereferenced, they will
; not be compatible types. This also checks that the safety flag is pointer
; carried to the referenced types.
%struct.test02a = type { %struct.test02a*, %struct.test02d* }
%struct.test02b = type { %struct.test02b*, %struct.test02d* }
%struct.test02c = type { %struct.test02c*, %struct.test02d* }
%struct.test02d = type { i32, i32 }
define internal void @test02(%struct.test02a* %pStructA, %struct.test02b* %pStructB, %struct.test02c* %pStructC) !dtrans_type !18 {

  %field.a.0 = getelementptr %struct.test02a, %struct.test02a* %pStructA, i64 0, i32 0
  %field.a.0.as.p64 = bitcast %struct.test02a** %field.a.0 to i64*
  %field.b.1 = getelementptr %struct.test02b, %struct.test02b* %pStructB, i64 0, i32 0
  %field.b.1.as.p64 = bitcast %struct.test02b** %field.b.1 to i64*
  %field.c.0 = getelementptr %struct.test02c, %struct.test02c* %pStructC, i64 0, i32 0
  %field.c.0.as.p64 = bitcast %struct.test02c** %field.c.0 to i64*
  br i1 undef, label %blockA, label %block_BorC

block_BorC:
  br i1 undef, label %blockB, label %blockC

blockA:
  br label %merge

blockB:
  br label %merge

blockC:
  br label %merge


merge:
  %addr = phi i64* [%field.a.0.as.p64, %blockA], [%field.b.1.as.p64, %blockB], [%field.c.0.as.p64, %blockC]
  store i64 0, i64* %addr
  ret void
}
; TODO: When store instructions are analyzed, the types for 'test02a',
; 'test02b', and 'test02c' should also be marked with 'Unsafe pointer
; store' because of the pointer operand used for the 'store' aliases
; multiple types.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Unsafe pointer merge{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02c
; CHECK: Unsafe pointer merge{{ *$}}

; Even though 'test02d' is not directly involved in the PHINode, it also
; needs to be marked due to the pointer carried safety rules because it is
; reachable from the unsafe types.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02d
; CHECK: Unsafe pointer merge{{ *$}}


; Merging the addresses of fields that do not represent pointers to aggregate
; types is not an "unsafe pointer merge", even if the field addresses come
; from different structures. The field info object will have marked the
; affected fields with the 'ComplexUse' flag in this case when the GEP is
; analyzed, which helps the transformations know that even though the
; structure is safe, transformations may be complex to do.
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i32, i32 }
%struct.test03c = type { i32, i32 }
define internal void @test03(%struct.test03a* %pStructA, %struct.test03b* %pStructB, %struct.test03c* %pStructC) !dtrans_type !19 {

  %pField.a.0 = getelementptr %struct.test03a, %struct.test03a* %pStructA, i64 0, i32 0
  %pField.b.1 = getelementptr %struct.test03b, %struct.test03b* %pStructB, i64 0, i32 1
  %pField.c.0 = getelementptr %struct.test03c, %struct.test03c* %pStructC, i64 0, i32 0
  br i1 undef, label %blockA, label %block_BorC

block_BorC:
  br i1 undef, label %blockB, label %blockC

blockA:
  br label %merge

blockB:
  br label %merge

blockC:
  br label %merge


merge:
  %addr = phi i32* [%pField.a.0, %blockA], [%pField.b.1, %blockB], [%pField.c.0, %blockC]
  store i32 0, i32* %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03c
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 3, !3, !4, !6, !8}  ; void (%struct.test01a*, %struct.test01b*, %struct.test01c*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01a*
!5 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!6 = !{!7, i32 1}  ; %struct.test01b*
!7 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!8 = !{!9, i32 1}  ; %struct.test01c*
!9 = !{!"R", %struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!10 = !{!11, i32 1}  ; %struct.test02a*
!11 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!12 = !{!13, i32 1}  ; %struct.test02d*
!13 = !{!"R", %struct.test02d zeroinitializer, i32 0}  ; %struct.test02d
!14 = !{!15, i32 1}  ; %struct.test02b*
!15 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!16 = !{!17, i32 1}  ; %struct.test02c*
!17 = !{!"R", %struct.test02c zeroinitializer, i32 0}  ; %struct.test02c
!18 = !{!"F", i1 false, i32 3, !3, !10, !14, !16}  ; void (%struct.test02a*, %struct.test02b*, %struct.test02c*)
!19 = !{!"F", i1 false, i32 3, !3, !20, !22, !24}  ; void (%struct.test03a*, %struct.test03b*, %struct.test03c*)
!20 = !{!21, i32 1}  ; %struct.test03a*
!21 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!22 = !{!23, i32 1}  ; %struct.test03b*
!23 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!24 = !{!25, i32 1}  ; %struct.test03c*
!25 = !{!"R", %struct.test03c zeroinitializer, i32 0}  ; %struct.test03c
!26 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!27 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!28 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!29 = !{!"S", %struct.test02a zeroinitializer, i32 2, !10, !12} ; { %struct.test02a*, %struct.test02d* }
!30 = !{!"S", %struct.test02b zeroinitializer, i32 2, !14, !12} ; { %struct.test02b*, %struct.test02d* }
!31 = !{!"S", %struct.test02c zeroinitializer, i32 2, !16, !12} ; { %struct.test02c*, %struct.test02d* }
!32 = !{!"S", %struct.test02d zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!33 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!34 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!35 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
