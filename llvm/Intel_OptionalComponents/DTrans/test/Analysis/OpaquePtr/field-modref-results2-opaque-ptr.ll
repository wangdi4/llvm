; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat=true -dtrans-fieldmodrefop-analysis -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat=true -passes='require<dtrans-fieldmodrefop-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck %s

; Test mod/ref query results for a function call that makes an indirect
; function call based on a function address passed in a parameter.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, ptr, i64 }

define internal void @testbase(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %fieldaddr = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr

  ; ModRef queries of this call site should identify the result as 'ModRef' because
  ; 'test01' will directly modify the field, and it will reach a function that is
  ; indirectly called that will reference the field.
  call void @test01()
  ret void
}

define internal void @test01() {
  %st = call ptr @malloc(i64 24)

  %f0 = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  store i32 8, ptr %f0

  %ar1_mem = call ptr @malloc(i64 64)
  %f1 = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  store ptr %ar1_mem, ptr %f1

  %f2 = getelementptr %struct.test01, ptr %st, i64 0, i32 2
  store i64 1, ptr %f2

  ; Calling this function with the function address should result in mod/ref
  ; queries that include the behavior of the indirect function call. Note, the
  ; analysis is not context sensitive to the call site for pass-through
  ; functions, so the behavior of all functions that may be passed into use01a
  ; will be merged together to produce the result.
  call void @use01a(ptr %st, ptr @filter01a)
  ret void
}

define void @use01a(ptr "intel_dtrans_func_index"="1" %st, ptr "intel_dtrans_func_index"="2" nocapture %filter) !intel.dtrans.func.type !9 {
  call void %filter(ptr %st), !intel_dtrans_type !6
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01a(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !10 {
  %fieldaddr = getelementptr %struct.test01, ptr %st, i64 0, i32 0
  %ld1 = load i32, ptr %fieldaddr
  ret void
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK: FieldModRefQuery: - ModRef     : [testbase]   %ld1 = load i32, ptr %fieldaddr, align 4 --   call void @test01()
; CHECK: FieldModRefQuery: - Ref        : [test01]   store i32 8, ptr %f0, align 4 --   call void @use01a(ptr %st, ptr @filter01a)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{!"F", i1 false, i32 1, !7, !4}  ; void (%struct.test01*)
!7 = !{!"void", i32 0}  ; void
!8 = !{!6, i32 1}  ; void (%struct.test01*)*
!9 = distinct !{!4, !8}
!10 = distinct !{!4}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i32*, i64 }

!intel.dtrans.types = !{!13}

