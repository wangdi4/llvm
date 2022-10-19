; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test detection of "Unsafe pointer merge" safety condition when using PHI nodes


; Merging pointers to different types of structures together leads to
; "Unsafe pointer merge"
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i32, i32 }
%struct.test01c = type { i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %a_in, %struct.test01b* "intel_dtrans_func_index"="2" %b_in, %struct.test01c* "intel_dtrans_func_index"="3" %c_in) !intel.dtrans.func.type !5 {
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
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: Safety data: Bad casting | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test01c


; This case is merging pointers to pointers of the aggregate types. This is
; also marked as unsafe because when the pointers are dereferenced, they will
; not be compatible types. This also checks that the safety flag is pointer
; carried to the referenced types.
%struct.test02a = type { %struct.test02a*, %struct.test02d* }
%struct.test02b = type { %struct.test02b*, %struct.test02d* }
%struct.test02c = type { %struct.test02c*, %struct.test02d* }
%struct.test02d = type { i32, i32 }
define internal void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStructA, %struct.test02b* "intel_dtrans_func_index"="2" %pStructB, %struct.test02c* "intel_dtrans_func_index"="3" %pStructC) !intel.dtrans.func.type !10 {

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
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02c
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02c

; Even though 'test02d' is not directly involved in the PHINode, it also
; needs to be marked due to the pointer carried safety rules because it is
; reachable from the unsafe types.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02d
; CHECK: Unsafe pointer merge{{ *$}}
; CHECK: End LLVMType: %struct.test02d


; Merging the addresses of fields that do not represent pointers to aggregate
; types is not an "unsafe pointer merge", even if the field addresses come
; from different structures. The field info object will have marked the
; affected fields with the 'ComplexUse' flag in this case when the GEP is
; analyzed, which helps the transformations know that even though the
; structure is safe, transformations may be complex to do.
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i32, i32 }
%struct.test03c = type { i32, i32 }
define internal void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStructA, %struct.test03b* "intel_dtrans_func_index"="2" %pStructB, %struct.test03c* "intel_dtrans_func_index"="3" %pStructC) !intel.dtrans.func.type !14 {

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
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03c
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03c


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = !{%struct.test01c zeroinitializer, i32 1}  ; %struct.test01c*
!5 = distinct !{!2, !3, !4}
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{%struct.test02d zeroinitializer, i32 1}  ; %struct.test02d*
!8 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!9 = !{%struct.test02c zeroinitializer, i32 1}  ; %struct.test02c*
!10 = distinct !{!6, !8, !9}
!11 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!12 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!13 = !{%struct.test03c zeroinitializer, i32 1}  ; %struct.test03c*
!14 = distinct !{!11, !12, !13}
!15 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!16 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!17 = !{!"S", %struct.test01c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!18 = !{!"S", %struct.test02a zeroinitializer, i32 2, !6, !7} ; { %struct.test02a*, %struct.test02d* }
!19 = !{!"S", %struct.test02b zeroinitializer, i32 2, !8, !7} ; { %struct.test02b*, %struct.test02d* }
!20 = !{!"S", %struct.test02c zeroinitializer, i32 2, !9, !7} ; { %struct.test02c*, %struct.test02d* }
!21 = !{!"S", %struct.test02d zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!22 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!23 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!24 = !{!"S", %struct.test03c zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20, !21, !22, !23, !24}
