; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that return values are marked as 'System object' for aggregate types
; returned by external functions.

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
@name = internal global i8* zeroinitializer, !intel_dtrans_type !2
@mode = internal global i8* zeroinitializer, !intel_dtrans_type !2
define void @test02() {
  %name_val = load i8*, i8** @name
  %mode_val = load i8*, i8** @mode
  %handle = call %struct._IO_FILE* @fopen(i8* %name_val, i8* %mode_val)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._IO_FILE
; CHECK: Safety data: System object{{ *$}}
; CHECK: End LLVMType: %struct._IO_FILE

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._IO_marker
; CHECK: Safety data: System object{{ *$}}
; CHECK: End LLVMType: %struct._IO_marker

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" %struct._IO_FILE* @fopen(i8* "intel_dtrans_func_index"="2", i8* "intel_dtrans_func_index"="3")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct._IO_marker zeroinitializer, i32 1}  ; %struct._IO_marker*
!4 = !{%struct._IO_FILE zeroinitializer, i32 1}  ; %struct._IO_FILE*
!5 = !{i64 0, i32 0}  ; i64
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 0}  ; i8
!8 = !{!"A", i32 1, !7}  ; [1 x i8]
!9 = !{!"A", i32 20, !7}  ; [20 x i8]
!10 = distinct !{!4, !2, !2}
!11 = !{!"S", %struct._IO_FILE zeroinitializer, i32 29, !1, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !3, !4, !1, !1, !5, !6, !7, !8, !2, !5, !2, !2, !2, !2, !5, !1, !9} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
!12 = !{!"S", %struct._IO_marker zeroinitializer, i32 3, !3, !4, !1} ; { %struct._IO_marker*, %struct._IO_FILE*, i32 }

!intel.dtrans.types = !{!11, !12}
