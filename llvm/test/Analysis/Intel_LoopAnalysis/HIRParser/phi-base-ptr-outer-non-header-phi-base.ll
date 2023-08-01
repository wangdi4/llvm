; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loopnest verifying we can handle non-header phi
; %p.merge (which has an AddRec form) by conservatively treating is as non-linear.

; Previously, we were compfailing on this phi.


; CHECK: + DO i1 = 0, 10, 1   <DO_LOOP>
; CHECK: |   if (%c != 0)
; CHECK: |   {
; CHECK: |      %p.merge = &((%p)[i1 + 1]);
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %p.merge = &((%p)[i1 + 1]);
; CHECK: |   }
; CHECK: |
; CHECK: |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK: |   |   (%p.merge)[i2] = i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @sub(ptr nocapture %p, i1 %c) {
entry:
  br label %for.body.outer

for.body.outer:
  %iv.outer =  phi i32 [ 0, %entry ], [ %iv.outer.inc, %latch ]
  %p.outer = phi ptr [ %p, %entry ], [ %p.merge, %latch ]
  br i1 %c, label %then, label %else

then:
  %p.then.inc = getelementptr inbounds i32, ptr %p.outer, i64 1
  br label %for.body.pre

else:
  %p.else.inc = getelementptr inbounds i32, ptr %p.outer, i64 1
  br label %for.body.pre

for.body.pre:
  %p.merge = phi ptr [ %p.then.inc, %then ], [ %p.else.inc, %else ]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %for.body.pre ], [ %inc, %for.body ]
  %p.addr.05 = phi ptr [ %p.merge, %for.body.pre ], [ %incdec.ptr, %for.body ]
  store i32 %i.06, ptr %p.addr.05, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %p.addr.05, i64 1
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 1000
  br i1 %exitcond, label %latch, label %for.body

latch:
  %iv.outer.inc = add nuw nsw i32 %iv.outer, 1
  %cond = icmp eq i32 %iv.outer, 10
  br i1 %cond, label %end, label %for.body.outer

end:                                          ; preds = %for.body
  ret i32 0
}

