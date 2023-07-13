; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-use-block-freq=false -disable-output 2>&1 | FileCheck %s

; This test verifies that frequencies of field accesses are computed. This test
; uses the flag -dtrans-use-block-freq=false to cause the counters to just use
; static instruction counts to keep the test from being affected by any
; changes to the block frequency class implementation.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks field frequencies of struct.test01.
; CHECK: Name: struct.test01
; CHECK: Frequency: 7
; CHECK: Frequency: 2
; CHECK: Frequency: 0
; CHECK: Total Frequency: 9
%struct.test01 = type { i32, i64, i32 }
@g_instance.test01 = internal unnamed_addr global %struct.test01 zeroinitializer

; Checks field frequencies of struct.test02.
; CHECK: Name: struct.test02
; CHECK: Frequency: 1
; CHECK: Frequency: 3
; CHECK: Total Frequency: 4
%struct.test02 = type { ptr, i64 }

; Checks field frequencies of struct.test03.
; CHECK: Name: struct.test03
; CHECK: Frequency: 5
; CHECK: Frequency: 3
; CHECK: Total Frequency: 8
%struct.test03 = type { ptr, ptr }

; Max Total Frequency of all structures
; CHECK: MaxTotalFrequency: 9

define void @foo(ptr "intel_dtrans_func_index"="1" %tp, ptr "intel_dtrans_func_index"="2" %tp2) !intel.dtrans.func.type !5 {
entry:
  %i = getelementptr inbounds %struct.test01, ptr %tp, i64 0, i32 0
  %0 = load i32, ptr %i, align 8	; test01 @ 0
  %1 = load i32, ptr %i, align 8	; test01 @ 0
  %2 = load i32, ptr %i, align 8	; test01 @ 0
  %3 = load i32, ptr %i, align 8	; test01 @ 0
  %4 = load i32, ptr %i, align 8	; test01 @ 0
  %j = getelementptr inbounds %struct.test02, ptr %tp2, i64 0, i32 1
  %5 = load i64, ptr %j, align 8	; test02 @ 1
  %c = getelementptr inbounds %struct.test01, ptr %tp, i64 0, i32 1
  store i64 0, ptr %c, align 8	; test01 @ 1
  %t = getelementptr inbounds %struct.test01, ptr %tp, i64 0, i32 0
  store i32 10, ptr %t, align 8	; test01 @ 0
  %h = getelementptr inbounds %struct.test01, ptr %tp, i64 0, i32 0
  store i32 20, ptr %h, align 4	; test01 @ 0
  store i64 30, ptr getelementptr (%struct.test01, ptr @g_instance.test01, i64 0, i32 1), align 4 ; test01 @ 1
  ret void
}

define void @bar(ptr "intel_dtrans_func_index"="1" %tp, ptr "intel_dtrans_func_index"="2" %tp2, ptr "intel_dtrans_func_index"="3" %tp3) !intel.dtrans.func.type !7 {
entry:
  %i = getelementptr inbounds %struct.test02, ptr %tp2, i64 0, i32 1
  %0 = load i64, ptr %i, align 8 ; test02 @ 1
  %j = getelementptr inbounds %struct.test02, ptr %tp2, i64 0, i32 0
  store ptr null,  ptr %j, align 4 ; test02 @ 0
  %k = getelementptr inbounds %struct.test03, ptr %tp3, i64 0, i32 1
  store ptr null,  ptr %k, align 4 ; test03 @ 1
  store ptr null,  ptr %k, align 4 ; test03 @ 1
  store ptr null,  ptr %k, align 4 ; test03 @ 1
  ret void
}

define void @baz(ptr "intel_dtrans_func_index"="1" %tp2, ptr "intel_dtrans_func_index"="2" %tp3) !intel.dtrans.func.type !8 {
entry:
  %i = getelementptr inbounds %struct.test02, ptr %tp2, i64 0, i32 1
  %0 = load i64, ptr %i, align 8 ; test02 @ 1
  %j = getelementptr inbounds %struct.test03, ptr %tp3, i64 0, i32 0
  store ptr null,  ptr %j, align 4 ; test03 @ 0
  %1 = load ptr, ptr %j, align 8 ; test03 @ 0
  %2 = load ptr, ptr %j, align 8 ; test03 @ 0
  %3 = load ptr, ptr %j, align 8 ; test03 @ 0
  %4 = load ptr, ptr %j, align 8 ; test03 @ 0
  ret void
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!3, !4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!3, !4, !6}
!8 = distinct !{!4, !6}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !2} ; { %struct.test01*, i64 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 2, !4, !3} ; { %struct.test02*, %struct.test01* }

!intel.dtrans.types = !{!9, !10, !11}
