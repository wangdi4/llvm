; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures
; that are not supported.
;
; %struct.test01 is not supported because it contains an array element.
; %struct.dep01 is not supported because there is a global variable of the type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, [2 x i16] }
%struct.dep01 = type { i64, ptr }
@glob = internal global %struct.dep01 zeroinitializer

define i32 @main() {
  %mem = call ptr @calloc(i64 10, i64 24)
  store ptr %mem, ptr getelementptr (%struct.dep01, ptr @glob, i64 0, i32 1)
  ret i32 0
}
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.dep01
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported structure element type: %struct.test01

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"A", i32 2, !4}  ; [2 x i16]
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i64, [2 x i16] }
!9 = !{!"S", %struct.dep01 zeroinitializer, i32 2, !2, !5} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!8, !9}
