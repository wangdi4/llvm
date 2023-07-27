; RUN: opt -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S %s | FileCheck %s
; Verify CG for RHS of <2> which is an HInst with an underlying GetElementPtrInst
;          BEGIN REGION { }
;<9>          + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<2>          |   %add.ptr = &((%add.ptr)[i1]);
;<3>          |   (%add.ptr)[0] = i1;
;<9>          + END LOOP
;          END REGION

; CHECK: region.0:
; CHECK: {{loop.[0-9]+:}}

; CG for RHS, &((%add.ptr)[i1]);
; CHECK: [[IV_LOAD:%.*]] = load i64, ptr %i1.i64
; CHECK: [[ARRAY_ADDR:%.*]] = getelementptr inbounds i64, ptr [[A_SYM:%t[0-9+]]]{{.*}}, i64 [[IV_LOAD]]
; We should be storing it into lhs, %add.ptr's symbase memory slot
; CHECK-NEXT: store ptr [[ARRAY_ADDR]], ptr [[A_SYM]]




; ModuleID = 'poly-ptr1-64.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %p, i64 %n) {
entry:
  %cmp.6 = icmp sgt i64 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi ptr [ %add.ptr, %for.body ], [ %p, %for.body.preheader ]
  %add.ptr = getelementptr inbounds i64, ptr %p.addr.07, i64 %i.08
  store i64 %i.08, ptr %add.ptr, align 8
  %inc = add nuw nsw i64 %i.08, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
