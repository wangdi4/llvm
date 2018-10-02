; RUN: opt -hir-ssa-deconstruction -disable-output -print-after=hir-runtime-dd -hir-runtime-dd < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   (%p)[i1].0 = i1;
; |   (%p)[i1].1 = null;
; |   (%q)[i1] = 0;
; + END LOOP
; END REGION

; CHECK: %mv.upper.base = &((i32*)(%p)[sext.i32.i64(%n) + -1].1);
; CHECK: %mv.test = &((%mv.upper.base)[1]) >=u &((%q)[0]);
; CHECK: %mv.test1 = &((%q)[sext.i32.i64(%n) + -1]) >=u &((%p)[0].0);
; CHECK: %mv.and = %mv.test  &&  %mv.test1;
; CHECK: if (%mv.and == 0)

;Module Before HIR; ModuleID = 'ptr-types-struct.c'
source_filename = "ptr-types-struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.T = type { i32, %struct.T1* }
%struct.T1 = type { i32, i32, i32 }

; Function Attrs: norecurse nounwind uwtable
define void @foo(%struct.T* nocapture %p, i32* nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %n, 0
  br i1 %cmp12, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %a = getelementptr inbounds %struct.T, %struct.T* %p, i64 %indvars.iv, i32 0
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %a, align 8
  %b = getelementptr inbounds %struct.T, %struct.T* %p, i64 %indvars.iv, i32 1
  store %struct.T1* null, %struct.T1** %b, align 8
  %arrayidx4 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  store i32 0, i32* %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


