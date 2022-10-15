; REQUIRES: asserts

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test is to verify field value collection of structure fields when using
; byte-flattened GEPs.

; Include a data layout so that padding will be inserted to align structure
; field members.
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Verify value tracking works when writes are done via byte-flattened GEP form.
%struct.test01 = type { i32, i64*, i16 } ; Offsets: 0, 8, 16
define "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !5 {
  %flat = call i8* @malloc(i64 24)
  %obj = bitcast i8* %flat to %struct.test01*

  %pA = getelementptr i8, i8* %flat, i32 0
  %pB = getelementptr i8, i8* %flat, i32 8
  %pC = getelementptr i8, i8* %flat, i32 16
  %pA.i32 = bitcast i8* %pA to i32*
  %pB.p64 = bitcast i8* %pB to i64**
  %pC.i16 = bitcast i8* %pC to i16*

  store i32 99, i32* %pA.i32
  store i64* null, i64** %pB.p64
  store i16 16384, i16* %pC.i16

  ret %struct.test01* %obj
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Single Value: i32 99
; CHECK:  1)Field LLVM Type: {{.*}}
; CHECK:    Single Value: {{.*}} null
; CHECK:  2)Field LLVM Type: i16
; CHECK:    Single Value: i16 16384
; CHECK:  Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test01


declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i64*, i16 }

!intel.dtrans.types = !{!8}
