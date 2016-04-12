
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s
; Verify CG for RHS of <2> which is an HInst with an underlying GetElementPtrInst
;          BEGIN REGION { }
;<9>          + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<2>          |   %add.ptr = &((%add.ptr)[i1]);
;<3>          |   (%add.ptr)[0] = i1;
;<9>          + END LOOP
;          END REGION

; CHECK: region:
; CHECK: {{loop.[0-9]+:}}

; CG for RHS, &((%add.ptr)[i1]);
; CHECK: [[IV_LOAD:%.*]] = load i64, i64* %i1.i64
; CHECK: [[ARRAY_ADDR:%.*]] = getelementptr inbounds i64, i64* [[A_SYM:%t[0-9+]]]{{.*}}, i64 [[IV_LOAD]]
; We should be storing it into lhs, %add.ptr's symbase memory slot
; CHECK-NEXT: store i64* [[ARRAY_ADDR]], i64** [[A_SYM]]




; ModuleID = 'poly-ptr1-64.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %p, i64 %n) {
entry:
  %cmp.6 = icmp sgt i64 %n, 0
  br i1 %cmp.6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.08 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %p.addr.07 = phi i64* [ %add.ptr, %for.body ], [ %p, %entry ]
  %add.ptr = getelementptr inbounds i64, i64* %p.addr.07, i64 %i.08
  store i64 %i.08, i64* %add.ptr, align 8
  %inc = add nuw nsw i64 %i.08, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}


