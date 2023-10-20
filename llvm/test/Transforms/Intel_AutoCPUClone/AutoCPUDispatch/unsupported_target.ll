; REQUIRES: asserts

; The test checks that compiler does not fail with the
; Assertion `I->Mangling != '\0' && "Processor dooesn't support function multiversion!"'
; When target CPU does not support multiversioning.

; RUN: opt < %s -passes=auto-cpu-clone -acd-enable-all -S 2>&1 |  FileCheck %s

source_filename = "test.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse noreturn nosync nounwind memory(none) uwtable
define void @s1_(ptr noalias nocapture readnone dereferenceable(4) %"s1_$OUTPUT", ptr noalias nocapture readnone dereferenceable(4) %"s1_$BB", ptr noalias nocapture readnone dereferenceable(4) %"s1_$TT") local_unnamed_addr #0 !llvm.auto.cpu.dispatch !1 {
; CHECK-LABEL: define void @s1_
; CHECK-SAME: (ptr noalias nocapture readnone dereferenceable(4) %"s1_$OUTPUT", ptr noalias nocapture readnone dereferenceable(4) %"s1_$BB", ptr noalias nocapture readnone dereferenceable(4) %"s1_$TT") local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  alloca_0:
; CHECK-NEXT:    br label [[DO_BODY4:%.*]]
; CHECK:       do.body4:
; CHECK-NEXT:    br label [[DO_BODY4]]
;
alloca_0:
  br label %do.body4

do.body4:                                         ; preds = %do.body4, %alloca_0
  br label %do.body4
}

attributes #0 = { nofree norecurse noreturn nosync nounwind memory(none) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{!"znver4"}
