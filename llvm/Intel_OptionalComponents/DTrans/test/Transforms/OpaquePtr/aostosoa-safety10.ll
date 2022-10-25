; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures
; used with unsupported bitcast instructions.
;
; %struct.test01 is not supported because bitcasts performed on the type.
; %struct.dep01 is not supported because there is a global variable of the type.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, i32 }
%struct.dep01 = type { i64, %struct.test01* }
@glob = internal global %struct.dep01 zeroinitializer

define i32 @main() {
  call void @test01()
  call void @test02()
  ret i32 0
}

define void @test01() {
  %mem = call i8* @calloc(i64 10, i64 24)
  %st = bitcast i8* %mem to %struct.test01*
  store %struct.test01* %st, %struct.test01** getelementptr (%struct.dep01, %struct.dep01* @glob, i64 0, i32 1)
  ret void
}

define void @test02() {
  %st = load %struct.test01*, %struct.test01** getelementptr (%struct.dep01, %struct.dep01* @glob, i64 0, i32 1)
  %cast = bitcast %struct.test01* %st to i8*
  %cast_back = bitcast i8* %cast to %struct.test01*
  %use = getelementptr %struct.test01, %struct.test01* %cast_back, i64 0, i32 1
  store i64 0, i64* %use
  ret void
}
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported safety data: %struct.dep01
; CHECK-DAG: AOS-to-SOA rejecting -- Unsupported bitcasts: %struct.test01

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0
declare !intel.dtrans.func.type !6 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!8 = !{!"S", %struct.dep01 zeroinitializer, i32 2, !2, !3} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!7, !8}
