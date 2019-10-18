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
;        b = (ordering[i] > b) ? b : ordering[i];
;        tmp = (ordering[i] <= b) ? tmp : i;
;    }
;    return tmp + b;
;}
;
; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @maxloc(i32 %m, i32* nocapture readonly %ordering) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i32 %m, 0
  br i1 %cmp25, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %tmp.027 = phi i32 [ 0, %for.body.preheader ], [ %2, %for.body ]
  %b.026 = phi i32 [ -111111111, %for.body.preheader ], [ %b.0., %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ordering, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, %b.026
  %b.0. = select i1 %cmp1, i32 %b.026, i32 %0
  %1 = trunc i64 %indvars.iv to i32
  %2 = select i1 %cmp1, i32 %1, i32 %tmp.027
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %b.0..lcssa = phi i32 [ %b.0., %for.body ]
  %.lcssa = phi i32 [ %2, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %b.0.lcssa = phi i32 [ -111111111, %entry ], [ %b.0..lcssa, %for.end.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %.lcssa, %for.end.loopexit ]
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

