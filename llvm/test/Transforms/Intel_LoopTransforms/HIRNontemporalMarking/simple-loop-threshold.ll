; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-nontemporal-marking" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

target triple = "x86_64-unknown-linux-gnu"

; Check to make sure that we convert to nontemporal for a simple loop.

; Before:
; <0>          BEGIN REGION { }
; <10>               + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; <4>                |   (%dest)[i1] = i1;
; <10>               + END LOOP
; <0>          END REGION

; Verify that pass is dumped with print-changed when it triggers.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRNontemporalMarking

define void @example(ptr %dest) "target-features"="+avx512f" {
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
