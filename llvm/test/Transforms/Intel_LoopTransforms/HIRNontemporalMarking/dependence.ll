; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -hir-ssa-deconstruction -hir-nontemporal-marking -print-after=hir-nontemporal-marking -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes="hir-ssa-deconstruction,hir-nontemporal-marking,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check that we will convert to nontemporal correctly in the case of various
; dependences: we should not convert for FLOW or OUTPUT dependences; though it
; is legal to convert in the presence of an ANTI dependence, we don't because
; there isn't expected to be a performance upside.

; Before:
; Function: read_after_write
;
; <0>          BEGIN REGION { }
; <13>               + DO i1 = 0, 6400000, 1   <DO_LOOP>
; <5>                |   (%dest)[i1] = i1;
; <6>                |   %val = (%dest)[i1 + -8];
; <7>                |   %recirc = %val  +  %recirc;
; <13>               + END LOOP
; <0>          END REGION
;
; Function: write_after_write
;
; <0>          BEGIN REGION { }
; <12>               + DO i1 = 0, 6400000, 1   <DO_LOOP>
; <5>                |   (%dest)[i1] = i1;
; <6>                |   (%dest)[i1 + -8] = 0;
; <12>               + END LOOP
; <0>          END REGION
;
; Function: write_after_read
;
; <0>          BEGIN REGION { }
; <12>               + DO i1 = 0, 6400000, 1   <DO_LOOP>
; <5>                |   %val = (%dest)[i1];
; <6>                |   (%dest)[i1 + -8] = %val;
; <12>               + END LOOP
; <0>          END REGION

define i64 @read_after_write(i64* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: read_after_write
;      CHECK: BEGIN REGION { }
; CHECK-NEXT:       + Ztt: No
; CHECK-NEXT:       + NumExits: 1
; CHECK-NEXT:       + Innermost: Yes
; CHECK-NEXT:       + HasSignedIV: Yes
; CHECK-NEXT:       + LiveIn symbases: 3, 8
; CHECK-NEXT:       + LiveOut symbases: 3
; CHECK-NEXT:       + Loop metadata: No
; CHECK-NEXT:       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; CHECK-NEXT:       |   (%dest)[i1] = i1;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1] inbounds  {sb:11}
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:8}
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; CHECK-NEXT:       |
; CHECK-NEXT:       |   %val = (%dest)[i1 + -8];
; CHECK-NEXT:       |   <LVAL-REG> NON-LINEAR i64 %val {sb:9}
; CHECK-NEXT:       |   <RVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1 + -8] inbounds  {sb:11}
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:8}
; CHECK-NEXT:       |
; CHECK-NEXT:       |   %recirc = %val  +  %recirc;
; CHECK-NEXT:       |   <LVAL-REG> NON-LINEAR i64 %recirc {sb:3}
; CHECK-NEXT:       |   <RVAL-REG> NON-LINEAR i64 %val {sb:9}
; CHECK-NEXT:       |   <RVAL-REG> NON-LINEAR i64 %recirc {sb:3}
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

entry:
  %dest.offset = getelementptr inbounds i64, i64* %dest, i64 -8
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %recirc = phi i64 [ 0, %entry ], [ %sum, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, i64* %dest, i64 %index
  %addr2 = getelementptr inbounds i64, i64* %dest.offset, i64 %index
  store i64 %index, i64* %addr, align 8
  %val = load i64, i64* %addr2, align 8
  %sum = add i64 %val, %recirc
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret i64 %sum
}

define void @write_after_write(i64* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: write_after_write
;      CHECK: BEGIN REGION { }
; CHECK-NEXT:       + Ztt: No
; CHECK-NEXT:       + NumExits: 1
; CHECK-NEXT:       + Innermost: Yes
; CHECK-NEXT:       + HasSignedIV: Yes
; CHECK-NEXT:       + LiveIn symbases: 7
; CHECK-NEXT:       + LiveOut symbases:
; CHECK-NEXT:       + Loop metadata: No
; CHECK-NEXT:       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; CHECK-NEXT:       |   (%dest)[i1] = i1;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1] inbounds  {sb:9}
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:7}
; CHECK-NEXT:       |   <RVAL-REG> LINEAR i64 i1 {sb:2}
; CHECK-NEXT:       |
; CHECK-NEXT:       |   (%dest)[i1 + -8] = 0;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1 + -8] inbounds
; CHECK-NOT:               !nontemporal
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:7}
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

entry:
  %dest.offset = getelementptr inbounds i64, i64* %dest, i64 -8
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, i64* %dest, i64 %index
  %addr2 = getelementptr inbounds i64, i64* %dest.offset, i64 %index
  store i64 %index, i64* %addr, align 8
  store i64 0, i64* %addr2, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

define void @write_after_read(i64* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: write_after_read
;      CHECK: BEGIN REGION { }
; CHECK-NEXT:       + Ztt: No
; CHECK-NEXT:       + NumExits: 1
; CHECK-NEXT:       + Innermost: Yes
; CHECK-NEXT:       + HasSignedIV: Yes
; CHECK-NEXT:       + LiveIn symbases: 7
; CHECK-NEXT:       + LiveOut symbases:
; CHECK-NEXT:       + Loop metadata: No
; CHECK-NEXT:       + DO i64 i1 = 0, 6400000, 1   <DO_LOOP>
; CHECK-NEXT:       |   %val = (%dest)[i1];
; CHECK-NEXT:       |   <LVAL-REG> NON-LINEAR i64 %val {sb:8}
; CHECK-NEXT:       |   <RVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1] inbounds  {sb:10}
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:7}
; CHECK-NEXT:       |
; CHECK-NEXT:       |   (%dest)[i1 + -8] = %val;
; CHECK-NEXT:       |   <LVAL-REG> {al:8}(LINEAR i64* %dest)[LINEAR i64 i1 + -8] inbounds
; CHECK-NOT:               !nontemporal
; CHECK-NEXT:       |      <BLOB> LINEAR i64* %dest {sb:7}
; CHECK-NEXT:       |   <RVAL-REG> NON-LINEAR i64 %val {sb:8}
; CHECK-NEXT:       |
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

entry:
  %dest.offset = getelementptr inbounds i64, i64* %dest, i64 -8
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %addr = getelementptr inbounds i64, i64* %dest, i64 %index
  %addr2 = getelementptr inbounds i64, i64* %dest.offset, i64 %index
  %val = load i64, i64* %addr, align 8
  store i64 %val, i64* %addr2, align 8
  %cond = icmp eq i64 %index, 6400000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}
