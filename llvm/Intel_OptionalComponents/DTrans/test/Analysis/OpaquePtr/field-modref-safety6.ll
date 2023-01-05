; REQUIRES: asserts

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; Check handling of address taken functions for field based Mod/Ref analysis.
; In this case, the address taken function is not allowed because it is passed
; as a parameter to another function, which stores the address to a global
; variable, which could allow the function to be called from some other
; function.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr, i64 }

@current_func = global ptr zeroinitializer, !intel_dtrans_type !7
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
  ret void
}

; Function pointer is not 'nocapture' in this case, and gets saved to memory,
; which will cause the structure fields that are accessible to be set to
; 'bottom'.
define void @use01a(ptr "intel_dtrans_func_index"="1" %st, ptr "intel_dtrans_func_index"="2" %filter) !intel.dtrans.func.type !8 {
  store ptr %filter, ptr @current_func
  call void %filter(ptr %st), !intel_dtrans_type !4
  ret void
}

; Function that will be address taken
define void @filter01a(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !9 {
  call void @read01.0(ptr %st)
  ret void
}

; Function that will be address taken
define void @filter01b(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !10 {
  call void @read01.1(ptr %st)
  ret void
}

; Function called by address taken function that accesses structure fields.
define void @read01.0(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !11 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr
  ret void
}

; Function called by address taken function that accesses structure fields.
define void @read01.1(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !12 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  %ld1 = load ptr, ptr %fieldaddr
  ret void
}

; Fields 0 and 1 are accessed from functions that reachable from the address
; taken functions, and the function pointers are captured into a memory
; location. This should result in these fields being marked as 'bottom'.

; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: bottom
; CHECK: 2)Field DTrans Type: i64
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = !{!4, i32 1}  ; void (%struct.test01*)*
!8 = distinct !{!6, !7}
!9 = distinct !{!6}
!10 = distinct !{!6}
!11 = distinct !{!6}
!12 = distinct !{!6}
!13 = !{i8 0, i32 1}  ; i8*
!14 = distinct !{!13}
!15 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i32*, i64 }

!intel.dtrans.types = !{!15}
