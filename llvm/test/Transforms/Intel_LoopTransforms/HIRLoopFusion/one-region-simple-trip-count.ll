; RUN: opt -hir-ssa-deconstruction -disable-output -hir-loop-fusion -print-after=hir-loop-fusion -hir-create-function-level-region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Different pragma trip count metadata in the loops will be removed after the loop fusion. Only the same trip count metadate will be kept
;
; INPUT:
;<0>       BEGIN REGION { }
;<29>            + DO i1 = 0, 99, 1   <DO_LOOP> <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 100> <min_trip_count = 60> <avg_trip_count = 70> <max_trip_count = 100>
;<5>             |   (%a)[i1] = i1;
;<29>            + END LOOP
;<29>
;<30>
;<30>            + DO i1 = 0, 99, 1   <DO_LOOP> <MAX_TC_EST = 100> <<LEGAL_MAX_TC = 100> min_trip_count = 50> <avg_trip_count = 80> <max_trip_count = 100>
;<17>            |   %1 = (%a)[i1];
;<20>            |   (%b)[i1] = %1 + 1;
;<30>            + END LOOP
;<30>
;<28>            ret ;
;<0>       END REGION
;
;*** IR Dump After HIR Loop Fusion ***
;
; CHECK:     BEGIN REGION { modified }
; CHECK:     DO i1 = 0, 99, 1   <DO_LOOP> <MAX_TC_EST = 100> <LEGAL_MAX_TC = 100> <max_trip_count = 100>
;
source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* nocapture %b, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv24
  %0 = trunc i64 %indvars.iv24 to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 100
  br i1 %exitcond26, label %for.cond.cleanup, label %for.body, !llvm.loop !0


for.cond.cleanup4:                                ; preds = %for.body5
  ret void

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %add, i32* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5, !llvm.loop !4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
!0 = distinct !{!0, !1, !2, !3}
!1 = !{!"llvm.loop.intel.loopcount_maximum", i32 100}
!2 = !{!"llvm.loop.intel.loopcount_minimum", i32 60}
!3 = !{!"llvm.loop.intel.loopcount_average", i32 70}
!4 = distinct !{!4, !5, !6, !7}
!5 = !{!"llvm.loop.intel.loopcount_maximum", i32 100}
!6 = !{!"llvm.loop.intel.loopcount_minimum", i32 50}
!7 = !{!"llvm.loop.intel.loopcount_average", i32 80}

