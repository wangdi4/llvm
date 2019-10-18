;
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -vplan-import-entities -vplan-use-entity-instr=true -disable-vplan-codegen -enable-mmindex -debug-only=parvec-analysis -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s
;
; Test for min/max+index is not recognized.
;
;CHECK:Idiom List
;CHECK-NEXT:  No idioms detected.
;
;int  maxloc (int m, int* ordering) {
;    int b = -111111111;
;    int tmp = 0;
;    int i;
;    for (i=0; i< m; i++) {
;        if (ordering[i] > b) {
;          b = ordering[i];
;          tmp = i;
;        }
;        else
;          tmp = 2*i;
;    }
;    return tmp + b;
;}
;
;
; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @maxloc(i32 %m, i32* nocapture readonly %ordering) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %m, 0
  br i1 %cmp14, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = add i32 %m, -1
  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %b.015 = phi i32 [ -111111111, %for.body.preheader ], [ %2, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ordering, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %1, %b.015
  %2 = select i1 %cmp1, i32 %1, i32 %b.015
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %cmp1.lcssa = phi i1 [ %cmp1, %for.body ]
  %.lcssa = phi i32 [ %2, %for.body ]
  %not.cmp1.le = xor i1 %cmp1.lcssa, true
  %mul.le = zext i1 %not.cmp1.le to i32
  %3 = shl nuw nsw i32 %0, %mul.le
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  %b.0.lcssa = phi i32 [ %.lcssa, %for.cond.for.end_crit_edge ], [ -111111111, %entry ]
  %tmp.0.lcssa = phi i32 [ %3, %for.cond.for.end_crit_edge ], [ 0, %entry ]
  %add = add nsw i32 %tmp.0.lcssa, %b.0.lcssa
  ret i32 %add
}

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

