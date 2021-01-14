; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis of calls to memset that take the address of an
; array of structure elements or pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Write fields of the structure within the array.
%struct.test01 = type { i64, i64 }
define void @test01() {
  %a = alloca [8 x [16384 x %struct.test01]]
  %a.p8 = bitcast [8 x [16384 x %struct.test01]]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %a.p8, i8 1, i64 2097152, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i64
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info: Written
; CHECK: Safety data: Local instance{{ *$}}


; Writing a pointer type, not the structure fields.
%struct.test02 = type { i64, i64 }
define void @test02() {
  %a = alloca [8 x [16384 x %struct.test02*]], !dtrans_type !2
  %a.p8 = bitcast [8 x [16384 x %struct.test02*]]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %a.p8, i8 1, i64 1048576, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i64
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Local pointer{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"A", i32 8, !3}  ; [8 x [16384 x %struct.test02*]]
!3 = !{!"A", i32 16384, !4}  ; [16384 x %struct.test02*]
!4 = !{!5, i32 1}  ; %struct.test02*
!5 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!6 = !{!"F", i1 false, i32 4, !7, !8, !9, !1, !10}  ; void (i8*, i8, i64, i1)
!7 = !{!"void", i32 0}  ; void
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{i8 0, i32 0}  ; i8
!10 = !{i1 0, i32 0}  ; i1
!11 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!13 = !{!"llvm.memset.p0i8.i64", !6}

!dtrans_types = !{!11, !12}
!dtrans_decl_types = !{!13}
