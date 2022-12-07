; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that safe GEP uses do not get marked with safety flags by the checks
; for 'Ambiguous GEP' or 'Bad pointer manipulation'. The cases are checking
; when the GEP is using the byte-flattened form.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Access to element of non-nested structure
%struct.test01 = type { i32, ptr, i32, i16, i16 } ; Offsets: 0, 8, 16, 20, 22
define internal void @test01() {
  %local = alloca %struct.test01

  %addr0 = getelementptr i8, ptr %local, i32 0
  %addr1 = getelementptr i8, ptr %local, i32 8
  %addr2 = getelementptr i8, ptr %local, i32 16
  %addr3 = getelementptr i8, ptr %local, i32 20
  %addr4 = getelementptr i8, ptr %local, i32 22
  call void @test01helper(ptr %addr0, ptr %addr1, ptr %addr2, ptr %addr3, ptr %addr4)
  ret void
}
define internal void @test01helper(ptr "intel_dtrans_func_index"="1" %arg0, ptr "intel_dtrans_func_index"="2" %arg1, ptr "intel_dtrans_func_index"="3" %arg2, ptr "intel_dtrans_func_index"="4" %arg3, ptr "intel_dtrans_func_index"="5" %arg4) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Local instance | Field address taken call{{ *$}}


; Access to element of nested structure
%struct.test02a = type { i16, i32 } ; Offset: 0, 4
%struct.test02b = type { i16, %struct.test02a } ; Offsets 0, 4
define internal void @test02() {
  %local = alloca %struct.test02b
  %faddr = getelementptr i8, ptr %local, i64 8
  store i32 0, ptr %faddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Nested structure | Local instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Contains nested structure | Local instance{{ *$}}


; Byte-flattened GEP that does not align to the start of a field, but is an
; alignment padding byte, where the result is passed to 'memset'.
; This is a special case that does not trigger the 'Bad pointer manipulation'
; safety flag.
%struct.test03 = type { i32, i16, i32 } ; Offsets: 0, 4, 8
define internal void @test03(i32 %x) {
  %local = alloca %struct.test03
  %pad_addr = getelementptr i8, ptr %local, i32 6
  call void @llvm.memset.p0i8.i64(ptr %pad_addr, i8 0, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; TODO: When memory intrinsics are analyzed, this will also get "Memfunc
; partial write" set.
; CHECK: Local instance{{ *$}}


declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4, !4, !4, !4, !4}
!6 = !{%struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!7 = distinct !{!4}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !2, !1, !3, !3} ; { i32, i64*, i32, i16, i16 }
!9 = !{!"S", %struct.test02a zeroinitializer, i32 2, !3, !1} ; { i16, i32 }
!10 = !{!"S", %struct.test02b zeroinitializer, i32 2, !3, !6} ; { i16, %struct.test02a }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !3, !1} ; { i32, i16, i32 }

!intel.dtrans.types = !{!8, !9, !10, !11}
