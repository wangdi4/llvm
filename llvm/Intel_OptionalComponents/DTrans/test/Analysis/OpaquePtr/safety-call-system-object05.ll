; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that types passed to file IO routines are marked as "System object"

%struct._ZTS8_IO_FILE._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque

@str01 = private constant [23 x i8] c"DUAL NETWORK SIMPLEX: \00"
define void @test01(%struct._ZTS8_IO_FILE._IO_FILE* "intel_dtrans_func_index"="1" %pFile) !intel.dtrans.func.type !12 {
  %tmp = call i64 @fwrite(i8* getelementptr ([23 x i8], [23 x i8]* @str01, i64 0, i64 0),
                           i64 22, i64 1, %struct._ZTS8_IO_FILE._IO_FILE* %pFile)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS8_IO_FILE._IO_FILE
; CHECK: Safety data: Address taken | System object{{ *$}}
; CHECK: End LLVMType: %struct._ZTS8_IO_FILE._IO_FILE


declare !intel.dtrans.func.type !13 i64 @fwrite(i8* "intel_dtrans_func_index"="1", i64, i64, %struct._ZTS8_IO_FILE._IO_FILE* "intel_dtrans_func_index"="2")

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}  ; %struct._ZTS10_IO_marker._IO_marker*
!4 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}  ; %struct._ZTS8_IO_FILE._IO_FILE*
!5 = !{i64 0, i32 0}  ; i64
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 0}  ; i8
!8 = !{!"A", i32 1, !7}  ; [1 x i8]
!9 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}  ; %struct._ZTS11_IO_codecvt._IO_codecvt*
!10 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}  ; %struct._ZTS13_IO_wide_data._IO_wide_data*
!11 = !{!"A", i32 20, !7}  ; [20 x i8]
!12 = distinct !{!4}
!13 = distinct !{!2, !4}
!14 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !1, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !3, !4, !1, !1, !5, !6, !7, !8, !2, !5, !9, !10, !4, !2, !5, !1, !11} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
!15 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 0} ; opaque
!16 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 0} ; opaque
!17 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 0} ; opaque

!intel.dtrans.types = !{!14, !15, !16, !17}
