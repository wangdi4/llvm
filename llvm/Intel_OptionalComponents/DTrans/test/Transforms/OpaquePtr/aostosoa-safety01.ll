; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures
; that do not meet the DTrans safety conditions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, i32 }
%struct.other01 = type { i32, i32, i32, i32 }
%struct.dep01 = type { i64, %struct.test01* }
@glob = internal global %struct.dep01 zeroinitializer

define i32 @main() {
  %mem = call i8* @calloc(i64 10, i64 24)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** getelementptr (%struct.dep01, %struct.dep01* @glob, i64 0, i32 1)
  call void @test01()
  ret i32 0
}

define void @test01() {
  %addr = getelementptr %struct.dep01, %struct.dep01* @glob, i64 0, i32 1
  %ptr = load %struct.test01*, %struct.test01** %addr
  ; Introduce an Ambiguous GEP safety violation for the types.
  %cast = bitcast %struct.test01* %ptr to %struct.other01*
  %field = getelementptr %struct.other01, %struct.other01* %cast, i64 0, i32 3
  store i32 0, i32* %field
  ret void
}
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.test01
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.other01
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.dep01

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!7 = !{!"S", %struct.other01 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!8 = !{!"S", %struct.dep01 zeroinitializer, i32 2, !2, !3} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!6, !7, !8}
