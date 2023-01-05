; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; an indirect call to an aligned memory allocation routine.

; CHECK: DTRANS Weak Align: inhibited -- May allocate alignment memory

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Address taken of new(unsigned int, align_val_t) routine.
@malloc_func = dso_local local_unnamed_addr global ptr @_ZnwmSt11align_val_t, align 8, !intel_dtrans_type !4

define internal void @test01() {
  %fn_addr = load ptr, ptr @malloc_func, align 8
  %call = tail call ptr %fn_addr(i64 64, i64 16), !intel_dtrans_type !1
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare !intel.dtrans.func.type !5  "intel_dtrans_func_index"="1" ptr @_ZnwmSt11align_val_t(i64, i64)

!1 = !{!"F", i1 false, i32 2, !2, !3, !3}  ; i8* (i64, i64)
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{i64 0, i32 0}  ; i64
!4 = !{!1, i32 1}  ; i8* (i64, i64)*
!5 = distinct !{!2}

!intel.dtrans.types = !{}

