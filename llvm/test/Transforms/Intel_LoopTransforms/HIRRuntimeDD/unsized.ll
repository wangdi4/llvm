; RUN: opt -hir-cost-model-throttling=0 -S -disable-output -disable-hir-runtime-dd-cost-model -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that loop will not be MV'ed because of %a is unsized.

; BEGIN REGION { }
;       + DO i1 = 0, 999, 1   <DO_LOOP>
;       |   + DO i2 = 0, 999, 1   <DO_LOOP>
;       |   |   %call = @bar(&((%a)[0]));
;       |   |   (%p)[i1][i2] = %call;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: Function: foo

; CHECK-NOT: if
; CHECK: DO i1
; CHECK: DO i2
; CHECK: END LOOP
; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type opaque

define void @foo(ptr %a, ptr nocapture %p, i32 %n) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv18 = phi i64 [ 0, %entry ], [ %indvars.iv.next19, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20 = icmp eq i64 %indvars.iv.next19, 1000
  br i1 %exitcond20, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %call = tail call i32 @bar(ptr %a)
  %arrayidx6 = getelementptr inbounds [10 x i32], ptr %p, i64 %indvars.iv18, i64 %indvars.iv
  store i32 %call, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

declare i32 @bar(ptr) local_unnamed_addr

