; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis
; for a case using a broker function.

; Pass function address to a broker function as arguments that are
; not the callback function, and not pass-through arguments. These
; functions should be treated as address-taken for the analysis, rather
; than as asafe calls.

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

  ; Based on the callback metadata descriptor, arguments 0 and 1, are not forwarded
  ; to the callback routine as nocapture arguments, and should cause fields accessed
  ; by them to be set to 'bottom'.
  tail call void (ptr, ptr, ptr, ...) @broker(
    ptr @filter01a,
    ptr @filter01b,
    ptr bitcast
      (ptr @use01a to ptr),
    i64 1,
    ptr %st
  )

  ret void
}

; Function that will be address taken because it is a callback for the broker
; function.
define void @use01a(ptr "intel_dtrans_func_index"="1" %in0, ptr "intel_dtrans_func_index"="2" %in1, i64 %in2, ptr "intel_dtrans_func_index"="3" %in3) !intel.dtrans.func.type !6 {
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01a(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !7 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01b(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !8 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 2
  store i64 0, ptr %fieldaddr
  ret void
}

declare !intel.dtrans.func.type !14 !callback !0 void @broker(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3", ...)
declare !intel.dtrans.func.type !16 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: 2)Field DTrans Type: i64
; CHECK: RWState: bottom
; CHECK: End LLVMType: %struct.test01

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = distinct !{!3, !3, !5}
!7 = distinct !{!5}
!8 = distinct !{!5}
!9 = !{!"F", i1 false, i32 1, !10, !5}  ; void (%struct.test01*)
!10 = !{!"void", i32 0}  ; void
!11 = !{!9, i32 1}  ; void (%struct.test01*)*
!12 = !{!"F", i1 true, i32 2, !10, !3, !3}  ; void (i32*, i32*, ...)
!13 = !{!12, i32 1}  ; void (i32*, i32*, ...)*
!14 = distinct !{!11, !11, !13}
!15 = !{i8 0, i32 1}  ; i8*
!16 = distinct !{!15}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 3, !2, !3, !4} ; { i32, i32*, i64 }

!intel.dtrans.types = !{!17}
