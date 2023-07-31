; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation" -print-changed -disable-output -hir-cost-model-throttling=0 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED
;
; %t.addr.040 should not be moved to postexit as it is conditionally executed.
;
;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<46>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
;<6>             |   if ((%A)[i1] > 0)
;<6>             |   {
;<15>            |      %2 = (%B)[i1 + 1];
;<17>            |      (%B)[i1 + 1] = %2 + 1;
;<21>            |      %.pre.pre-phi = &((%B)[i1]);
;<22>            |      %arrayidx16.pre-phi = &((%B)[i1]);
;<23>            |      if ((%B)[i1] > 0)
;<23>            |      {
;<24>            |         goto L;
;<23>            |      }
;<6>             |   }
;<6>             |   else
;<6>             |   {
;<11>            |      %.pre.pre-phi = &((%B)[i1]);
;<6>             |   }
;<29>            |   %t.addr.040 = i1 + %b + 1;
;<30>            |   %arrayidx16.pre-phi = &((%.pre.pre-phi)[0]);
;<32>            |   L:
;<34>            |   %5 = (%A)[i1];
;<37>            |   (%A)[i1] = %5 * i1;
;<38>            |   %7 = (%arrayidx16.pre-phi)[0];
;<40>            |   (%arrayidx16.pre-phi)[0] = %7 + 1;
;<46>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK-NOT:    modified
;

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLastValueComputation

;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture %B, i32 %t, i32 %N, i32 %b) local_unnamed_addr #0 {
entry:
  %cmp39 = icmp sgt i32 %N, 0
  br i1 %cmp39, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %L, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %1, %L ]
  %t.addr.040 = phi i32 [ %t, %for.body.preheader ], [ %t.addr.1, %L ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  %1 = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp1, label %if.then, label %for.body.if.end8_crit_edge

for.body.if.end8_crit_edge:                       ; preds = %for.body
  %.pre45 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  br label %if.end8

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %B, i64 %1
  %2 = load i32, ptr %arrayidx3, align 4, !tbaa !2
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx5, align 4, !tbaa !2
  %cmp6 = icmp sgt i32 %3, 0
  br i1 %cmp6, label %L, label %if.end8

if.end8:                                          ; preds = %for.body.if.end8_crit_edge, %if.then
  %.pre.pre-phi = phi ptr [ %.pre45, %for.body.if.end8_crit_edge ], [ %arrayidx5, %if.then ]
  %4 = trunc i64 %1 to i32
  %add10 = add i32 %4, %b
  br label %L

L:                                                ; preds = %if.then, %if.end8
  %arrayidx16.pre-phi = phi ptr [ %arrayidx5, %if.then ], [ %.pre.pre-phi, %if.end8 ]
  %t.addr.1 = phi i32 [ %t.addr.040, %if.then ], [ %add10, %if.end8 ]
  %5 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %6 = trunc i64 %indvars.iv to i32
  %mul = mul nsw i32 %5, %6
  store i32 %mul, ptr %arrayidx, align 4, !tbaa !2
  %7 = load i32, ptr %arrayidx16.pre-phi, align 4, !tbaa !2
  %inc17 = add nsw i32 %7, 1
  store i32 %inc17, ptr %arrayidx16.pre-phi, align 4, !tbaa !2
  %exitcond = icmp eq i64 %1, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %L
  %t.addr.1.lcssa = phi i32 [ %t.addr.1, %L ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.addr.0.lcssa = phi i32 [ %t, %entry ], [ %t.addr.1.lcssa, %for.end.loopexit ]
  %arrayidx19 = getelementptr inbounds i32, ptr %A, i64 5
  %8 = load i32, ptr %arrayidx19, align 4, !tbaa !2
  %mul20 = mul nsw i32 %8, %t.addr.0.lcssa
  %add21 = add nsw i32 %mul20, 1
  %arrayidx22 = getelementptr inbounds i32, ptr %B, i64 5
  store i32 %add21, ptr %arrayidx22, align 4, !tbaa !2
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7c6feb7b57a1fb6fb93d81865c58ebbd9b8f4401) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d8f3695a53bfbe128ff8045c6194697bfb1e4375)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
