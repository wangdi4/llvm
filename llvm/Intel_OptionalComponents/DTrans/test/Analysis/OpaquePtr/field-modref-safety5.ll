; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis.
; In this case, the address taken function is allowed because it is passed as a
; parameter to another function. This should not cause the fields to be set to
; 'bottom' because it will still be analyzable based on the reachability
; analysis by tracking the call made using the argument in the called function.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr, i64 }

define internal void @test01() {
  %st_mem = call ptr @malloc(i64 24)
  %st = bitcast ptr %st_mem to ptr

  %f0 = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i32 8, ptr %f0

  %ar1_mem = call ptr @malloc(i64 64)
  %ar1_mem2 = bitcast ptr %ar1_mem to ptr
  %f1 = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  store ptr %ar1_mem2, ptr %f1

  %f2 = getelementptr %struct.test01, ptr %st, i64 0, i32 2
  store i64 1, ptr %f2

  ; Create an address taken condition by passing function addresses to a
  ; function.
  call void @use01a(ptr %st, ptr @filter01a)
  call void @use01a(ptr %st, ptr @filter01b)
  call void @use01b(ptr %st)
  ret void
}

define void @use01a(ptr "intel_dtrans_func_index"="1" %st, ptr "intel_dtrans_func_index"="2" nocapture %filter) !intel.dtrans.func.type !8 {
  ; Indirect call to the function passed in via a parameter.
  call void %filter(ptr %st), !intel_dtrans_type !5
  ret void
}

define void @use01b(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !9 {
  ; Perform direct calls to the address taken functions to be ensure the
  ; code handles these.
  call void @filter01a(ptr %st)
  call void @filter01b(ptr %st)
  ret void
}

; Function that will be address taken
define void @filter01a(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !10 {
  call void @read01.0(ptr %st)
  ret void
}

; Function that will be address taken
define void @filter01b(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !11 {
  call void @read01.1(ptr %st)
  ret void
}

; Function called by an address taken function that accesses structure fields.
define void @read01.0(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !12 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr
  ret void
}

; Function called by an address taken function that accesses structure fields.
define void @read01.1(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !13 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  %ld1 = load ptr, ptr %fieldaddr
  ret void
}

; The use of an address taken function should not result in all the fields of
; the structure being marked as 'bottom'.

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: 2)Field DTrans Type: i64
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !15 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = !{!"F", i1 false, i32 1, !6, !4}  ; void (%struct.test01*)
!6 = !{!"void", i32 0}  ; void
!7 = !{!5, i32 1}  ; void (%struct.test01*)*
!8 = distinct !{!4, !7}
!9 = distinct !{!4}
!10 = distinct !{!4}
!11 = distinct !{!4}
!12 = distinct !{!4}
!13 = distinct !{!4}
!14 = !{i8 0, i32 1}  ; i8*
!15 = distinct !{!14}
!16 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i32*, i64 }

!intel.dtrans.types = !{!16}
