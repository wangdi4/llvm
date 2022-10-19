; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis.
; In this case, the address taken function is not allowed because it is passed
; as a parameter to an indirect function call.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i32*, i64 }
@funcAddr = global void (%struct.test01*, void (%struct.test01*)*)* zeroinitializer, !intel_dtrans_type !9

define internal void @test01() {
  %st_mem = call i8* @malloc(i64 24)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 1, i64* %f2

  %fptr = load void (%struct.test01*, void (%struct.test01*)*)*, void (%struct.test01*, void (%struct.test01*)*)** @funcAddr

  ; Pass the address of a function to an indirect function call.
  call void %fptr(%struct.test01* %st, void (%struct.test01*)* @filter01a), !intel_dtrans_type !4
  ret void
}

define void @use01a(%struct.test01* "intel_dtrans_func_index"="1" %st, void (%struct.test01*)* "intel_dtrans_func_index"="2" nocapture %filter) !intel.dtrans.func.type !10 {
  call void %filter(%struct.test01* %st), !intel_dtrans_type !7
  ret void
}

; Function that will be address taken.
define void @filter01a(%struct.test01* "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !11 {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

; Field 0 is accessed by a function that is address taken, so should get marked
; as 'bottom'.

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: 2)Field DTrans Type: i64
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{!"F", i1 false, i32 2, !5, !6, !8}  ; void (%struct.test01*, void (%struct.test01*)*)
!5 = !{!"void", i32 0}  ; void
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01*)
!8 = !{!7, i32 1}  ; void (%struct.test01*)*
!9 = !{!4, i32 1}  ; void (%struct.test01*, void (%struct.test01*)*)*
!10 = distinct !{!6, !8}
!11 = distinct !{!6}
!12 = !{i8 0, i32 1}  ; i8*
!13 = distinct !{!12}
!14 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i32*, i64 }

!intel.dtrans.types = !{!14}
