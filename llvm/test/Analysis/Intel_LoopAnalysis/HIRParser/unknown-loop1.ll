; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check that we correctly deconstruct and parse the unknown loop. The liveout copy "%hir.de.ssa.copy0.out = %0" is essential when deconstructing the header phi %0 otheriwse its use in the bottom test will cause live range violation.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   while.body:
; CHECK: |   %next_node.0261.out = &((%next_node.0261)[0]);
; CHECK: |   %hir.de.ssa.copy0.out = %0;
; CHECK: |   %shl47 = 1  <<  trunc.i64.i5(%hir.de.ssa.copy0.out);
; CHECK: |   %and49 = %in3  &&  %shl47;
; CHECK: |   %2 = (%and49 == 0) ? &((%next_node.0261.out)[0].2) : &((%next_node.0261.out)[0].3);
; CHECK: |   %3 = (%2)[0];
; CHECK: |   %4 = (%3)[0].1;
; CHECK: |   %0 = %4;
; CHECK: |   %next_node.0261 = &((%3)[0]);
; CHECK: |   if (%hir.de.ssa.copy0.out >u %4)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto while.body;
; CHECK: |   }
; CHECK: + END LOOP


%struct.trie_node = type { i64, i64, %struct.trie_node*, %struct.trie_node* }

define void @foo(i64 %in1, %struct.trie_node* %in2, i64 %in3) {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %0 = phi i64 [ %4, %while.body ], [ %in1, %entry ]
  %next_node.0261 = phi %struct.trie_node* [ %3, %while.body ], [ %in2, %entry ]
  %sh_prom46 = trunc i64 %0 to i32
  %1 = and i32 %sh_prom46, 31
  %shl47 = shl i32 1, %1
  %conv48 = sext i32 %shl47 to i64
  %and49 = and i64 %in3, %conv48
  %tobool50 = icmp eq i64 %and49, 0
  %llink54 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %next_node.0261, i64 0, i32 2
  %rlink52 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %next_node.0261, i64 0, i32 3
  %2 = select i1 %tobool50, %struct.trie_node** %llink54, %struct.trie_node** %rlink52
  %3 = load %struct.trie_node*, %struct.trie_node** %2, align 8
  %cmpbit41 = getelementptr inbounds %struct.trie_node, %struct.trie_node* %3, i64 0, i32 1
  %4 = load i64, i64* %cmpbit41, align 8
  %cmp42 = icmp ugt i64 %0, %4
  br i1 %cmp42, label %while.body, label %for.cond56.preheader.loopexit

for.cond56.preheader.loopexit:
  ret void
}
