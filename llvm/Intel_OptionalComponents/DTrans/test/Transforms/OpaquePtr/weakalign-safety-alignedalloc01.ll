; REQUIRES: asserts
; RUN: opt < %s -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of a
; direct call to an aligned memory allocation routine.

; CHECK: DTRANS Weak Align: inhibited -- May allocate alignment memory

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.std::nothrow_t" = type { i8 }
@nt = global %"struct.std::nothrow_t" zeroinitializer

define internal void @test01() {
  ; Call to new(unsigned int, align_val_t, nothrow)
  %p = call ptr @_ZnwmSt11align_val_tRKSt9nothrow_t(i64 256, i64 32, ptr @nt)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare !intel.dtrans.func.type !4  "intel_dtrans_func_index"="1" ptr @_ZnwmSt11align_val_tRKSt9nothrow_t(i64, i64, ptr "intel_dtrans_func_index"="2")

!1 = !{i8 0, i32 0}  ; i8
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%"struct.std::nothrow_t" zeroinitializer, i32 1}  ; %"struct.std::nothrow_t"*
!4 = distinct !{!2, !3}
!5 = !{!"S", %"struct.std::nothrow_t" zeroinitializer, i32 1, !1} ; { i8 }

!intel.dtrans.types = !{!5}

