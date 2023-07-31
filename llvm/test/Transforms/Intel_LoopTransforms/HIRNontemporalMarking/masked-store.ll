; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -hir-nontemporal-cacheline-count=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details -disable-output < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check to make sure masked stores are not converted to nontemporal.

; Before:
; <0>          BEGIN REGION { modified }
; <20>               + DO i1 = 0, 6399999, 16   <DO_LOOP> <auto-vectorized> <novectorize>
; <25>               |   %.vec = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> != 0;
; <28>               |   (<16 x double>*)(%dest)[i1] = 0.000000e+00, Mask = @{%.vec};
; <20>               + END LOOP
; <0>          END REGION

define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: example
;     CHECK: BEGIN REGION { modified }
;     CHECK:       + DO i64 i1 = 0, 6399999, [[#]]   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NOT:           !nontemporal
;     CHECK:       + END LOOP
; CHECK-NOT:          @llvm.x86.sse.sfence();
;     CHECK: END REGION

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %skipstore ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds double, ptr %dest, i64 %index
  %odd = trunc i64 %index to i1
  br i1 %odd, label %dostore, label %skipstore

dostore:
  store double 0.0, ptr %addr, align 8
  br label %skipstore

skipstore:
  %cond = icmp eq i64 %index.next, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
