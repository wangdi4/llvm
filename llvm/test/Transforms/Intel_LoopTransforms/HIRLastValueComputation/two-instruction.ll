; RUN: opt -passes="hir-ssa-deconstruction,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;
;<0>       BEGIN REGION { }
;<19>            + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<5>             |   %0 = (%A)[i1 + 1];
;<9>             |   (%A)[i1] = %0 + %b;
;<12>            |   %add5 = 2 * %b * i1  +  100;
;<13>            |   %add6 = i1  +  2;
;<19>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:        |   %0 = (%A)[i1 + 1];
; CHECK:        |   (%A)[i1] = %0 + %b;
; CHECK:        + END LOOP
; CHECK:           %add5 = -2 * %b + 2 * (%N * %b)  +  100;
; CHECK:           %add6 = %N + -1  +  2;
; CHECK:  END REGION
;
; ModuleID = 'foo3.ll'
source_filename = "foo3.ll"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(ptr %A, i32 %N, i32 %b) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp slt i32 0, %N
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %for.body ]
  %add = add nsw i32 %i.02, 1
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %add1 = add nsw i32 %0, %b
  %idxprom2 = sext i32 %i.02 to i64
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 %idxprom2
  store i32 %add1, ptr %arrayidx3, align 4
  %mul = mul nsw i32 %b, %i.02
  %mul4 = mul nsw i32 %mul, 2
  %add5 = add nsw i32 %mul4, 100
  %add6 = add nsw i32 %i.02, 2
  %cmp = icmp slt i32 %add, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %split = phi i32 [ %add5, %for.body ]
  %split3 = phi i32 [ %add6, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %t1.0.lcssa = phi i32 [ %split, %for.cond.for.end_crit_edge ], [ undef, %entry ]
  %t2.0.lcssa = phi i32 [ %split3, %for.cond.for.end_crit_edge ], [ undef, %entry ]
  %add7 = add nsw i32 %t2.0.lcssa, %t1.0.lcssa
  ret i32 %add7
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7c6feb7b57a1fb6fb93d81865c58ebbd9b8f4401) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d8f3695a53bfbe128ff8045c6194697bfb1e4375)"}
