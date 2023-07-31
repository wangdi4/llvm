; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -disable-output -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Verify that two loops will not be fused over unsafe user call in preheader.

;       BEGIN REGION { }
;          + DO i1 = 0, 127, 1   <DO_LOOP>
;          |   + DO i2 = 0, %size.inner + -1, 1   <DO_LOOP>
;          |   |   (%ary)[i2] = i2;
;          |   + END LOOP
;          |
;          |
;          |      @bar();
;          |   + DO i2 = 0, %size.inner + -1, 1   <DO_LOOP>
;          |   |   (%ary)[i2] = i2;
;          |   + END LOOP
;          + END LOOP
;       END REGION

; CHECK: DO i1
; CHECK:   DO i2 = 0, %size.inner + -1, 1 
; CHECK:    @bar() 
; CHECK:   DO i2 = 0, %size.inner + -1, 1


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %ary, ptr nocapture %ary2, i64 %size.inner) {
entry:
  br label %loop.outer

loop.outer:
  %i = phi i64 [ 0, %entry ], [ %i.next, %loop.outer.latch ]
  %ztt = icmp eq i64 %size.inner, 0
  br i1 %ztt, label %loop.outer.latch, label %preheader

preheader:
   br label %for.body

for.body:
  %j = phi i64 [ 0, %preheader ], [ %j.next, %for.body ]
  %ptr = getelementptr inbounds i64, ptr %ary, i64 %j
  store i64 %j, ptr %ptr, align 8
  %j.next = add nsw i64 %j, 1
  %cmp.j = icmp ne i64 %j.next, %size.inner
  br i1 %cmp.j, label %for.body, label %postexit

postexit:
   br label %preheader2

preheader2:
  tail call void (...) @bar()
  br label %for.body2

for.body2:
  %k = phi i64 [ 0, %preheader2 ], [ %k.next, %for.body2 ]
  %ptr2 = getelementptr inbounds i64, ptr %ary, i64 %k
  store i64 %k, ptr %ptr2, align 8
  %k.next = add nsw i64 %k, 1
  %cmp.k = icmp ne i64 %k.next, %size.inner
  br i1 %cmp.k, label %for.body2, label %postexit2

postexit2:
  br label %loop.outer.latch

loop.outer.latch:
  %i.next = add nsw i64 %i, 1
  %cmp.i = icmp slt i64 %i.next, 128
  br i1 %cmp.i, label %loop.outer, label %exit

exit:
  ret void
}

declare dso_local void @bar(...) local_unnamed_addr

