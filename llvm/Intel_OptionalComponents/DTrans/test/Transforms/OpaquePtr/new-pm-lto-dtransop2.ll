; Test DTrans integration in the new pass manager.
;
; Test that the DTrans analysis results get invalidated with the
; new pass manager when passes make changes to the structure types.
; This test should apply the field deletion and AOS-to-SOA transformations,
; which should cause the analysis to be invalidated after these passes run.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-verify -enable-npm-dtrans -dtransop-allow-typed-pointers  -debug-pass-manager -whole-program-assume -intel-libirc-allowed -passes='lto<O2>' -internalize-public-api-list main -S  %s 2>&1 | FileCheck %s

; CHECK: Running pass: dtransOP::CommuteCondOPPass on [module]
; CHECK: Running analysis: dtransOP::DTransSafetyAnalyzer on [module]
; CHECK: Running pass:  dtransOP::DeleteFieldOPPass on [module]
; CHECK: Invalidating analysis: dtransOP::DTransSafetyAnalyzer on [module]
; CHECK-NEXT: Running pass: dtransOP::ReorderFieldsOPPass on [module]
; CHECK:Running analysis: dtransOP::DTransSafetyAnalyzer on [module]
; CHECK: Running pass: dtransOP::AOSToSOAOPPass on [module]
; CHECK: Invalidating analysis: dtransOP::DTransSafetyAnalyzer on [module]
; CHECK: Running pass: dtransOP::DynClonePass on [module]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @main() {
  %mem = call i8* @calloc(i64 10, i64 48)
  %st = bitcast i8* %mem to %struct.test*
  %res = call i32 @doSomething(%struct.test* %st)
  ret i32 %res
}

define i32 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !5 {
  %cast = bitcast %struct.test* %st to i8*

  %f0 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 0
  %f1 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 1
  %f2 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 2
  %f3 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 3
  %f4 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 4
  %f5 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 5
  %f6 = getelementptr %struct.test, %struct.test* %st, i64 0, i32 6

  ; Write all fields
  store i32 0, i32* %f0
  store i64 0, i64* %f1
  store i32 0, i32* %f2
  store i32 0, i32* %f3
  store i16 0, i16* %f4
  store i64 0, i64* %f5
  store i64 0, i64* %f6

  ; Read some fields
  %v32a = load i32, i32* %f0
  %v32b = load i32, i32* %f3

  %v64a = load i64, i64* %f1
  %v64b = load i64, i64* %f6

  %v16a = load i16, i16* %f4

  %t1 = add i32 %v32a, %v32b
  %t2 = add i64 %v64a, %v64b
  %t3 = zext i32 %t1 to i64
  %t4 = add i64 %t2, %t3
  %t5 = zext i16 %v16a to i64
  %t6 = add i64 %t4, %t5
  %low = trunc i64 %t6 to i32
  ret i32 %low
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!8}
