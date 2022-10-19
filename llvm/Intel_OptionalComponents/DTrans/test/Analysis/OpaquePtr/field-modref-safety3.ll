; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis.
; Address taken functions that get stored to memory cause the state of fields
; accessed by the function to be set to 'bottom' because the analysis is not
; able to track where the function may be called.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i32* }
@funcptr = global void (%struct.test01*)* @read01.0, !intel_dtrans_type !6

define internal void @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  call void @read01.0(%struct.test01* %st)

  ret void
}

define void @read01.0(%struct.test01* "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !7 {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}
; Field 0 should get set to 'bottom' because it is used by the address taken
; function. Field 1 is still OK because it is not accessed by the function.

; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01*)
!4 = !{!"void", i32 0}  ; void
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{!3, i32 1}  ; void (%struct.test01*)*
!7 = distinct !{!5}
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }

!intel.dtrans.types = !{!10}
