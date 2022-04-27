; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -disable-output -print-after=hir-runtime-dd -hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -disable-output 2>&1 | FileCheck %s

; BEGIN REGION { }
;      + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;      |   %0 = (%q)[i1];
;      |   (%p)[i1] = %0;
;      |   if (%n == 4)
;      |   {
;      |      %1 = (%q)[i1];
;      |      (%q)[i1] = %1 + 1;
;      |   }
;      + END LOOP
; END REGION

; CHECK: if
; CHECK: DO i1
; CHECK: DO i1

;Module Before HIR; ModuleID = 'ifs.c'
source_filename = "ifs.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %n, 0
  br i1 %cmp14, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp3 = icmp eq i32 %n, 4
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %0, i32* %arrayidx2, align 4
  br i1 %cmp3, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %1 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


