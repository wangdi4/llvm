; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that types passed to file IO routines are marked as "System object"

%struct._ZTS8_IO_FILE._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque

@str01 = private constant [23 x i8] c"DUAL NETWORK SIMPLEX: \00"
define void @test01(%struct._ZTS8_IO_FILE._IO_FILE* %pFile) !dtrans_type !16 {
  %tmp = call i64 @fwrite(i8* getelementptr ([23 x i8], [23 x i8]* @str01, i64 0, i64 0),
                           i64 22, i64 1, %struct._ZTS8_IO_FILE._IO_FILE* %pFile)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct._ZTS8_IO_FILE._IO_FILE
; CHECK: Safety data: Address taken | System object{{ *$}}


declare i64 @fwrite(i8*, i64, i64, %struct._ZTS8_IO_FILE._IO_FILE*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!4, i32 1}  ; %struct._ZTS10_IO_marker._IO_marker*
!4 = !{!"R", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 0}  ; %struct._ZTS10_IO_marker._IO_marker
!5 = !{!6, i32 1}  ; %struct._ZTS8_IO_FILE._IO_FILE*
!6 = !{!"R", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 0}  ; %struct._ZTS8_IO_FILE._IO_FILE
!7 = !{i64 0, i32 0}  ; i64
!8 = !{i16 0, i32 0}  ; i16
!9 = !{i8 0, i32 0}  ; i8
!10 = !{!"A", i32 1, !9}  ; [1 x i8]
!11 = !{!12, i32 1}  ; %struct._ZTS11_IO_codecvt._IO_codecvt*
!12 = !{!"R", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 0}  ; %struct._ZTS11_IO_codecvt._IO_codecvt
!13 = !{!14, i32 1}  ; %struct._ZTS13_IO_wide_data._IO_wide_data*
!14 = !{!"R", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 0}  ; %struct._ZTS13_IO_wide_data._IO_wide_data
!15 = !{!"A", i32 20, !9}  ; [20 x i8]
!16 = !{!"F", i1 false, i32 1, !17, !5}  ; void (%struct._ZTS8_IO_FILE._IO_FILE*)
!17 = !{!"void", i32 0}  ; void
!18 = !{!"F", i1 false, i32 4, !7, !2, !7, !7, !5}  ; i64 (i8*, i64, i64, %struct._ZTS8_IO_FILE._IO_FILE*)
!19 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !1, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !2, !3, !5, !1, !1, !7, !8, !9, !10, !2, !7, !11, !13, !5, !2, !7, !1, !15} ; { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._ZTS10_IO_marker._IO_marker*, %struct._ZTS8_IO_FILE._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._ZTS11_IO_codecvt._IO_codecvt*, %struct._ZTS13_IO_wide_data._IO_wide_data*, %struct._ZTS8_IO_FILE._IO_FILE*, i8*, i64, i32, [20 x i8] }
!20 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 0} ; opaque
!21 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 0} ; opaque
!22 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 0} ; opaque
!23 = !{!"fwrite", !18}

!dtrans_types = !{!19, !20, !21, !22}
!dtrans_decl_types = !{!23}
