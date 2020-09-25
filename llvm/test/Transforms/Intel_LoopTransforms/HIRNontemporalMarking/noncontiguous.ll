; RUN: opt -enable-intel-advanced-opts -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-nontemporal-marking -print-after=hir-nontemporal-marking -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check to make sure that we do not convert non-contiguous accesses.

; Before:
; <0>          BEGIN REGION { }
; <10>               + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; <4>                |   (%dest)[i1][0] = i1;
; <10>               + END LOOP
; <0>          END REGION

define void @example([64 x i64]* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: example
;      CHECK: BEGIN REGION { }
;      CHECK:       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; CHECK-NEXT:       |   (%dest)[i1][0] = i1;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR [64 x i64]* %dest)[LINEAR i64 i1][i64 0] inbounds
;  CHECK-NOT:              !nontemporal
; CHECK-NEXT:       |      <BLOB> LINEAR [64 x i64]* %dest
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 i1
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds [64 x i64], [64 x i64]* %dest, i64 %index, i64 0
  store i64 %index, i64* %addr, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
