; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of a
; direct call to an aligned memory allocation routine that is marked as
; returning aligned memory via the allockind attribute.

; CHECK: DTRANS Weak Align: inhibited -- May allocate alignment memory

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.std::nothrow_t" = type { i8 }
@nt = global %"struct.std::nothrow_t" zeroinitializer

define internal void @test01() {
  %p = call ptr @aligned_alloc(i64 32, i64 512)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare !intel.dtrans.func.type !3 noalias ptr  @aligned_alloc(i64 allocalign, i64) #0

attributes #0 = { allockind("alloc,uninitialized,aligned") allocsize(1) "alloc-family"="malloc" }

!1 = !{i8 0, i32 0}  ; i8
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = !{!"S", %"struct.std::nothrow_t" zeroinitializer, i32 1, !1} ; { i8 }

!intel.dtrans.types = !{!4}

