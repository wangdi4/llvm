; REQUIRES: asserts

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-use-block-freq=false -disable-output 2>&1 | FileCheck %s


; This test verifies that frequencies of field accesses are computed from
; memset/memcpy calls. This test  uses the flag -dtrans-use-block-freq=false
; to cause the counters to just use static instruction counts to keep the test
; from being affected by any changes to the block frequency class implementation.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks field frequencies of struct.test01.
; CHECK: Name: struct.test01
; CHECK: Frequency: 1
; CHECK: Frequency: 2
; CHECK: Frequency: 3
; CHECK: Frequency: 2
; CHECK: Frequency: 2
; CHECK: Total Frequency: 10
%struct.test01 = type { i32, i32, i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  ; Write fields 0 - 4
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  %pStart = bitcast i32* %pField0 to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart, i8 1, i64 20, i1 false)

  ; Write fields 1 - 2
  %pField1 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pStart1 = bitcast i32* %pField1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart1, i8 2, i64 8, i1 false)

  ; Write fields 2 - 4
  %pField2 = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 2
  %pStart2 = bitcast i32* %pField2 to i8*
  call void @llvm.memset.p0i8.i64(i8* %pStart2, i8 3, i64 12, i1 false)

  ret void
}

; Checks field frequencies of struct.test02.
; CHECK: Name: struct.test02
; CHECK: Frequency: 2
; CHECK: Frequency: 2
; CHECK: Frequency: 1
; CHECK: Total Frequency: 5
%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStructA, %struct.test02* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  ; Write fields 0 - 2
  %pFieldA = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)

  ; Write fields 0 - 1
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)

  ret void
}

declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !8 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6, !6}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!9, !10}
