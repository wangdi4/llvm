; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis
; when the address taken function does not access the structure field, but calls
; a routine that accesses a structure field. This should cause the field to be
; set to 'bottom'.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr }
@funcptr = global ptr @func_at, !intel_dtrans_type !6

define internal void @test01() {
  %st_mem = call ptr @malloc(i64 16)
  %st = bitcast ptr %st_mem to ptr

  %f0 = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i32 8, ptr %f0

  %ar1_mem = call ptr @malloc(i64 64)
  %ar1_mem2 = bitcast ptr %ar1_mem to ptr
  %f1 = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  store ptr %ar1_mem2, ptr %f1

  call void @read01.0(ptr %st)
  ret void
}

define void @func_at(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !7 {
  call void @read01.0(ptr %st)
  ret void
}

define void @read01.0(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !8 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr
  ret void
}
; Field 0 should get set to bottom because it is used by a function reachable
; from the address taken function.

; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01*)
!4 = !{!"void", i32 0}  ; void
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{!3, i32 1}  ; void (%struct.test01*)*
!7 = distinct !{!5}
!8 = distinct !{!5}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }

!intel.dtrans.types = !{!11}
