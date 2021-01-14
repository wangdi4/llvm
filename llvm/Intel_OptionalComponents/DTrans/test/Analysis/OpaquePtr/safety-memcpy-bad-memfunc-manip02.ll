; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-outofboundsok=true -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -whole-program-assume -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -whole-program-assume -dtrans-outofboundsok=false -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F
; RUN: opt -whole-program-assume -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F

; Test case where "Bad memfunc manipulation" occurs on an array element within a structure.
;
; When -dtrans-outofboundsok=true, this should trigger the "Bad memfunc
; manipulation" flag because we treat that IR as possibly allowing access to go
; out of bounds.
;
; When -dtrans-outofboundsok=false, the safety flag is not set because we assume
; that the source rules prevent a valid access from crossing element boundaries.

%struct.test01a = type { i64, float, %struct.test01b }
%struct.test01b = type { i64, [10 x i8] }
@var01a = internal global %struct.test01a zeroinitializer
@str = internal global [4 x i8] zeroinitializer
define void @test01(i64 %arg)  {
  ; Here we cannot prove that the destination stays within the array bounds.
  %pDst = getelementptr %struct.test01a, %struct.test01a* @var01a, i64 0, i32 2, i32 1, i64 %arg
  %pSrc = bitcast [4 x i8]* @str to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01a
; CHECK_OOB_T: Safety data: Bad pointer manipulation | Global instance | Bad memfunc manipulation | Contains nested structure{{ *$}}
; CHECK_OOB_F: Safety data: Global instance | Contains nested structure{{ *$}}

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: Name: struct.test01b
; CHECK_OOB_T: Safety data: Bad pointer manipulation | Global instance | Bad memfunc manipulation | Nested structure{{ *$}}
; CHECK_OOB_F: Safety data: Global instance | Nested structure{{ *$}}


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)


!1 = !{i64 0, i32 0}  ; i64
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{!"A", i32 10, !5}  ; [10 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{!"F", i1 false, i32 4, !7, !8, !8, !1, !9}  ; void (i8*, i8*, i64, i1)
!7 = !{!"void", i32 0}  ; void
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{i1 0, i32 0}  ; i1
!10 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i64, float, %struct.test01b }
!11 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }
!12 = !{!"llvm.memcpy.p0i8.p0i8.i64", !6}

!dtrans_types = !{!10, !11}
!dtrans_decl_types = !{!12}
