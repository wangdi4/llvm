; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + DO i1 = 0, 511, 1   <DO_LOOP>
; |   (%a)[i1] = 1;
; + END LOOP
;
;
; + DO i1 = 0, 510, 1   <DO_LOOP>
; |   (%a)[i1] = 2;
; + END LOOP
;
;
; + DO i1 = 0, 511, 1   <DO_LOOP>
; |   %0 = (%a)[i1];
; |   %conv = sitofp.i32.float(%0);
; |   (%b)[i1] = %conv;
; + END LOOP
;
; ret ;
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 510, 1
; CHECK-NOT: DO i
; CHECK: |   (%a)[i1] = 1;
; CHECK-NOT: DO i
; CHECK: |   (%a)[i1] = 2;
; CHECK: |   %0 = (%a)[i1];
; CHECK: |   %conv = sitofp.i32.float(%0);
; CHECK: |   (%b)[i1] = %conv;
; CHECK: + END LOOP
;
;
; CHECK: + DO i1 = 0, 0, 1
; CHECK: |   (%a)[i1 + 511] = 1;
; CHECK: |   %0 = (%a)[i1 + 511];
; CHECK: |   %conv = sitofp.i32.float(%0);
; CHECK: |   (%b)[i1 + 511] = %conv;
; CHECK: + END LOOP
;
; CHECK: END REGION

;Module Before HIR; ModuleID = 'fuse2.c'
source_filename = "fuse2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %a, ptr nocapture %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv41 = phi i64 [ 0, %entry ], [ %indvars.iv.next42, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv41
  store i32 1, ptr %arrayidx, align 4
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 512
  br i1 %exitcond43, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  br label %for.body15

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv38 = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next39, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv38
  store i32 2, ptr %arrayidx7, align 4
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40 = icmp eq i64 %indvars.iv.next39, 511
  br i1 %exitcond40, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup14:                               ; preds = %for.body15
  ret void

for.body15:                                       ; preds = %for.body15, %for.cond.cleanup4
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup4 ], [ %indvars.iv.next, %for.body15 ]
  %arrayidx17 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx17, align 4
  %conv = sitofp i32 %0 to float
  %arrayidx19 = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  store float %conv, ptr %arrayidx19, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 512
  br i1 %exitcond, label %for.cond.cleanup14, label %for.body15
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


