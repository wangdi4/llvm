; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that return values are marked as 'System object' for aggregate types
; returned by external functions.

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
@name = internal global i8* zeroinitializer, !dtrans_type !2
@mode = internal global i8* zeroinitializer, !dtrans_type !2
define void @test02() {
  %name_val = load i8*, i8** @name
  %mode_val = load i8*, i8** @mode
  %handle = call %struct._IO_FILE* @fopen(i8* %name_val, i8* %mode_val)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct._IO_FILE
; CHECK: Safety data: System object{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct._IO_marker
; CHECK: Safety data: System object{{ *$}}

declare %struct._IO_FILE* @fopen(i8*, i8*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!4, i32 1}  ; %struct._IO_marker*
!4 = !{!"R", %struct._IO_marker zeroinitializer, i32 0}  ; %struct._IO_marker
!5 = !{!6, i32 1}  ; %struct._IO_FILE*
!6 = !{!"R", %struct._IO_FILE zeroinitializer, i32 0}  ; %struct._IO_FILE
!7 = !{i64 0, i32 0}  ; i64
!8 = !{i16 0, i32 0}  ; i16
!9 = !{i8 0, i32 0}  ; i8
!10 = !{!"A", i32 1, !9}  ; [1 x i8]
!11 = !{!"A", i32 20, !9}  ; [20 x i8]
!12 = !{!"S", %struct._IO_FILE zeroinitializer, i32 29, !1, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !3, !5, !1, !1, !7, !8, !9, !10, !2, !7, !2, !2, !2, !2, !7, !1, !11} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
!13 = !{!"S", %struct._IO_marker zeroinitializer, i32 3, !3, !5, !1} ; { %struct._IO_marker*, %struct._IO_FILE*, i32 }

!dtrans_types = !{!12, !13}
