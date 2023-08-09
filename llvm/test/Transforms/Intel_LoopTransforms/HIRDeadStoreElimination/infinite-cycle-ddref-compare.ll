; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that CanonExprUtils::compare(Type*, Type*) does not get into a cycle while comparing two cyclic types like the linked lists list1 and list2 below.
; This was happening when DDRefGatherer called from dead store elimination was trying to sort the refs.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   if (%t4 == 4)
; CHECK: |   {
; CHECK: |      if ((ptr)(%t3)[i1].0 == &((%t1)[0]))
; CHECK: |      {
; CHECK: |         (ptr)(%t3)[i1].0 = &((%t2)[0]);
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

%list1 = type { ptr }
%list2 = type { ptr }
%union1 = type { i64 }
%union2 = type { %union1 }

define void @foo(ptr %t1, ptr %t2, ptr %t3, i8 %t4, i64 %n) {
entry:
  br label %loop

loop:                                              ; preds = %latch, %t773
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %latch ]
  %cmp = icmp eq i8 %t4, 4
  br i1 %cmp, label %then1, label %latch

then1:                                              ; preds = %loop
  %gep = getelementptr inbounds %union2, ptr %t3, i64 %iv, i32 0
  %cast1 = bitcast ptr %gep to ptr
  %ld = load ptr, ptr %cast1, align 8
  %cmp1 = icmp eq ptr %ld, %t1
  br i1 %cmp1, label %then2, label %latch

then2:                                              ; preds = %then1
  %cast2 = bitcast ptr %gep to ptr
  store ptr %t2, ptr %cast2, align 8
  br label %latch

latch:                                              ; preds = %then2, %then1, %loop
  %iv.next = add nuw nsw i64 %iv, 1
  %latch.cmp = icmp eq i64 %iv.next, %n
  br i1 %latch.cmp, label %exit, label %loop

exit:
  ret void
}
