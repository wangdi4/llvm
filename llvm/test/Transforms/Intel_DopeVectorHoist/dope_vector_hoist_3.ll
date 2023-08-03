; This test verifies that DopeVectorHoist is not triggered because
; @sub_ is not marked as Fortran routine.

; REQUIRES: asserts
; RUN: opt < %s -passes=dopevectorhoist -debug-only=dopevectorhoist -dopevector-hoist-enable=true -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; CHECK: DopeVectorHoist for sub_  Skipped (Not Fortran)

; CHECK: bb_then:
; CHECK: %A_elem_addr_1 = getelementptr float, ptr %A_addr_fetch_1, i64 32
; CHECK: %B_elem_addr_1 = getelementptr float, ptr %B_addr_fetch_1, i64 34

; CHECK: bb_else:
; CHECK: %A_elem_addr_2 = getelementptr float, ptr %A_addr_fetch_2, i64 32
; CHECK: %B_elem_addr_2 = getelementptr float, ptr %B_addr_fetch_2, i64 34


; Function Attrs: nofree norecurse nounwind uwtable
define void @sub_(ptr noalias nocapture dereferenceable(96) "assumed_shape" "ptrnoalias" %"sub_$A", ptr noalias nocapture dereferenceable(96) "assumed_shape" "ptrnoalias" %"sub_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub_$I") local_unnamed_addr #0 {

bb_entry:
  br label %bb_if

bb_if:
  %"sub_$I_fetch164" = load i32, ptr %"sub_$I", align 1
  %rel165 = icmp slt i32 %"sub_$I_fetch164", 0
  br i1 %rel165, label %bb_then, label %bb_else

bb_then:                                        ; preds = %bb_entry
  %A_addr_1 = getelementptr %"QNCA_a", ptr %"sub_$A", i64 0, i32 0
  %A_addr_fetch_1 = load ptr, ptr %A_addr_1, align 1
  %A_elem_addr_1 = getelementptr float, ptr %A_addr_fetch_1, i64 32
  %B_addr_1 = getelementptr %"QNCA_a", ptr %"sub_$B", i64 0, i32 0
  %B_addr_fetch_1 = load ptr, ptr %B_addr_1, align 1
  %B_elem_addr_1 = getelementptr float, ptr %B_addr_fetch_1, i64 34
  br label %bb_end

bb_else:                                        ; preds = %bb17_entry
  %A_addr_2 = getelementptr %"QNCA_a", ptr %"sub_$A", i64 0, i32 0
  %A_addr_fetch_2 = load ptr, ptr %A_addr_2, align 1
  %A_elem_addr_2 = getelementptr float, ptr %A_addr_fetch_2, i64 32
  %B_addr_2 = getelementptr %"QNCA_a", ptr %"sub_$B", i64 0, i32 0
  %B_addr_fetch_2 = load ptr, ptr %B_addr_2, align 1
  %B_elem_addr_2 = getelementptr float, ptr %B_addr_fetch_2, i64 34
  br label %bb_end

bb_end:                                       ; preds = %bb_then, %bb_else
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}

!0 = !{}
