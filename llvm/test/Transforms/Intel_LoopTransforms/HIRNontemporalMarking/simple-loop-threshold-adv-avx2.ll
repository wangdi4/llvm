; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -mtriple=i686-- -mattr=+avx2 -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check to make sure that we convert to nontemporal for a simple loop
; when compiled for -enable-intel-advanced-opts and -mattr=+avx2.

; Before:
; <0>          BEGIN REGION { }
; <10>               + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; <4>                |   (%dest)[i1] = i1;
; <10>               + END LOOP
; <0>          END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

define void @example(ptr %dest) {
; CHECK-LABEL: example
;      CHECK: BEGIN REGION { modified }
; CHECK-NEXT:       + Ztt: No
; CHECK-NEXT:       + NumExits: 1
; CHECK-NEXT:       + Innermost: Yes
; CHECK-NEXT:       + HasSignedIV: Yes
; CHECK-NEXT:       + LiveIn symbases: 6
; CHECK-NEXT:       + LiveOut symbases:
; CHECK-NEXT:       + Loop metadata: No
; CHECK-NEXT:       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; CHECK-NEXT:       |   (%dest)[i1] = i1;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR ptr %dest)[LINEAR i64 i1] inbounds  !nontemporal
; CHECK-NEXT:       |      <BLOB> LINEAR ptr %dest {sb:6}
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT:          @llvm.x86.sse.sfence();
; CHECK-NEXT: END REGION

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, ptr %dest, i64 %index
  store i64 %index, ptr %addr, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
