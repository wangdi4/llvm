; RUN: opt -hir-ssa-deconstruction -disable-output -disable-hir-runtime-dd-cost-model -print-after=hir-runtime-dd -hir-runtime-dd < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -disable-output -disable-hir-runtime-dd-cost-model < %s 2>&1 | FileCheck %s

; Source:
; void foo(int *a, int *b, int n, int stride) {
;   for( int i = 0; i < n; i++ ) {
;     a[stride*i] = b[i];
;   }
; }

; HIR:
; BEGIN REGION
;   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;   |   %1 = (%b)[i1];
;   |   (%a)[sext.i32.i64(%stride) * i1] = %1;
;   + END LOOP
; END REGION

; CHECL: Function
; CHECK: BEGIN REGION
; CHECK:      %mv.test = &((%b)[sext.i32.i64(%n) + -1]) >=u &((%a)[-1 * smin(0, sext.i32.i64(%stride)) + (sext.i32.i64(%n) * smin(0, sext.i32.i64(%stride)))]);
; CHECK:      %mv.test1 = &((%a)[-1 * smax(0, sext.i32.i64(%stride)) + (sext.i32.i64(%n) * smax(0, sext.i32.i64(%stride)))]) >=u &((%b)[0]);
; CHECK:      %mv.and = %mv.test  &  %mv.test1;
; CHECK:      if (%mv.and == 0)
; CHECK: END REGION

;Module Before HIR; ModuleID = 'unknown_blob.c'
source_filename = "unknown_blob.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %a, i32* nocapture readonly %b, i32 %n, i32 %stride) local_unnamed_addr #0 {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %stride to i64
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %2 = mul nsw i64 %indvars.iv, %0
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %2
  store i32 %1, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


