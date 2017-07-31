; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll 2>&1 | FileCheck %s

; Check unrolling of profitable unknown loop.

; CHECK: Dump Before

; CHECK:      + UNKNOWN LOOP i1
; CHECK-NEXT: |   <i1 = 0>
; CHECK-NEXT: |   while.body:
; CHECK-NEXT: |   %hir.de.ssa.copy0.out = %0;
; CHECK-NEXT: |   %shl47 = 1  <<  trunc.i64.i5(%0);
; CHECK-NEXT: |   %and49 = %in3  &&  %shl47;
; CHECK-NEXT: |   %2 = (%and49 == 0) ? &((%next_node.0261)[i1].2) : &((%next_node.0261)[i1].3);
; CHECK-NEXT: |   %3 = (%2)[0];
; CHECK-NEXT: |   %4 = (%3)[0].1;
; CHECK-NEXT: |   %0 = %4;
; CHECK-NEXT: |   %next_node.0261 = &((%3)[0]);
; CHECK-NEXT: |   if (%hir.de.ssa.copy0.out >u %4)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      <i1 = i1 + 1>
; CHECK-NEXT: |      goto while.body;
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP

; CHECK: Dump After

; Check for the first unrolled iteration and the last iteration with the occurence of the IV and bottom test.

; CHECK:      + UNKNOWN LOOP i1
; CHECK-NEXT: |   <i1 = 0>
; CHECK-NEXT: |   while.body:
; CHECK-NEXT: |   %hir.de.ssa.copy0.out = %0;
; CHECK-NEXT: |   %shl47 = 1  <<  trunc.i64.i5(%0);
; CHECK-NEXT: |   %and49 = %in3  &&  %shl47;
; CHECK-NEXT: |   %2 = (%and49 == 0) ? &((%next_node.0261)[8 * i1].2) : &((%next_node.0261)[8 * i1].3);
; CHECK-NEXT: |   %3 = (%2)[0];
; CHECK-NEXT: |   %4 = (%3)[0].1;
; CHECK-NEXT: |   %0 = %4;
; CHECK-NEXT: |   %next_node.0261 = &((%3)[0]);
; CHECK-NEXT: |   if (%hir.de.ssa.copy0.out <=u %4)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      goto [[LOOPEXIT:loopexit.*]];
; CHECK-NEXT: |   }

; CHECK:      |   %2 = (%and49 == 0) ? &((%next_node.0261)[8 * i1 + 7].2) : &((%next_node.0261)[8 * i1 + 7].3);
; CHECK-NEXT: |   %3 = (%2)[0];
; CHECK-NEXT: |   %4 = (%3)[0].1;
; CHECK-NEXT: |   %0 = %4;
; CHECK-NEXT: |   %next_node.0261 = &((%3)[0]);
; CHECK-NEXT: |   if (%hir.de.ssa.copy0.out >u %4)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      <i1 = i1 + 1>
; CHECK-NEXT: |      goto while.body;
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP
; CHECK:      [[LOOPEXIT]]:


%struct.trie_node = type { i64, i64, %struct.trie_node*, %struct.trie_node* }

define void @foo(i64 %in1, %struct.trie_node* %in2, i64 %in3) {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %0 = phi i64 [ %4, %while.body ], [ %in1, %entry ]
  %next_node.0261 = phi %struct.trie_node* [ %3, %while.body ], [ %in2, %entry ]
  %iv = phi i64 [ %iv.next, %while.body ], [ 0, %entry ]
  %sh_prom46 = trunc i64 %0 to i32
  %1 = and i32 %sh_prom46, 31
  %shl47 = shl i32 1, %1
  %conv48 = sext i32 %shl47 to i64
  %and49 = and i64 %in3, %conv48
  %tobool50 = icmp eq i64 %and49, 0
  %llink54 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %next_node.0261, i64 %iv, i32 2
  %rlink52 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %next_node.0261, i64 %iv, i32 3
  %2 = select i1 %tobool50, %struct.trie_node** %llink54, %struct.trie_node** %rlink52
  %3 = load %struct.trie_node*, %struct.trie_node** %2, align 8
  %cmpbit41 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %3, i64 0, i32 1
  %iv.next = add i64 %iv, 1
  %4 = load i64, i64* %cmpbit41, align 8
  %cmp42 = icmp ugt i64 %0, %4
  br i1 %cmp42, label %while.body, label %for.cond56.preheader.loopexit

for.cond56.preheader.loopexit:
  ret void
}
