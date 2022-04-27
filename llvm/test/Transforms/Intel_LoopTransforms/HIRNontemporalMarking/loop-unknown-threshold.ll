; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-nontemporal-marking -print-after=hir-nontemporal-marking -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Do not convert if we don't know the size of the loop.

; Before:
; <0>          BEGIN REGION { }
; <10>               + DO i64 i1 = 0, %N, 1   <DO_LOOP>
; <4>                |   (%dest)[i1] = i1;
; <10>               + END LOOP
; <0>          END REGION

define void @example(i64* %dest, i64 %N) "target-features"="+avx512f" {
; CHECK-LABEL: example
;      CHECK: BEGIN REGION { }
; CHECK-NEXT:       + Ztt: No
; CHECK-NEXT:       + NumExits: 1
; CHECK-NEXT:       + Innermost: Yes
; CHECK-NEXT:       + HasSignedIV: No
; CHECK-NEXT:       + LiveIn symbases: 4, 7
; CHECK-NEXT:       + LiveOut symbases:
; CHECK-NEXT:       + Loop metadata: No
; CHECK-NEXT:       + DO i64 i1 = 0, %N, 1   <DO_LOOP>
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 %N {sb:4}
; CHECK-NEXT:       |
; CHECK-NEXT:       |   (%dest)[i1] = i1;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1] inbounds  {sb:9}
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:7}
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, i64* %dest, i64 %index
  store i64 %index, i64* %addr, align 8
  %cond = icmp eq i64 %index, %N
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
