;
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -vplan-import-entities -vplan-use-entity-instr=true -disable-vplan-codegen -enable-mmindex=1  -disable-nonlinear-mmindex=0 -debug-only=LoopVectorizationPlanner -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -vplan-import-entities -vplan-use-entity-instr=true -disable-vplan-codegen -enable-mmindex=1  -disable-nonlinear-mmindex=1 -debug-only=parvec-analysis -vplan-force-vf=4 -S < %s 2>&1 | FileCheck --check-prefixes=DISABLED %s
;CHECK-LABEL: VPlan IR for: After insertion VPEntities instructions
;CHECK: Reduction list
;CHECK-NEXT: signed (SIntMax) Start: i32 [[BEST:%best.[0-9]+]] Exit: i32 [[BEST_EXIT:%vp[0-9]+]]
;CHECK-NEXT:   Linked values:{{.*}}
;CHECK-EMPTY:
;CHECK-NEXT: signed (SIntMin) Start: i32 [[TMP:%tmp.[0-9]+]] Exit: i32 [[TMP_EXIT:%vp[0-9]+]]
;CHECK-NEXT:   Linked values:{{.*}}
;CHECK-NEXT:  Parent exit: i32 [[BEST_EXIT]]
;CHECK-NEXT: signed (SIntMin) Start: i32 [[VAL:%val.[0-9]+]] Exit: i32 [[VAL_EXIT:%vp[0-9]+]]
;CHECK-NEXT:   Linked values:{{.*}}
;CHECK-NEXT:  Parent exit: i32 [[BEST_EXIT]]
;CHECK-EMPTY:
;CHECK-NEXT: Induction list
;CHECK-NEXT: IntInduction(+) Start: i32 0 Step: i32 1 BinOp: i32 {{%vp[0-9]+}} = add i32 {{%vp[0-9]+}} i32 {{%vp[0-9]+}}
;
;CHECK:  BB3 (BP: NULL) :
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = add
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = reduction-init i32 [[BEST]]
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = reduction-init i32 [[TMP]]
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = reduction-init i32 [[VAL]]
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = induction-init{add} i32 0 i32 1
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = induction-init-step{add} i32 1
;
;CHECK:  BB5 (BP: NULL) :
;CHECK-NEXT:  i32 [[BEST_FINAL:%vp[0-9]+]] = reduction-final{u_smax} i32 [[BEST_EXIT]]
;CHECK-NEXT:  i32 [[TMP_FINAL:%vp[0-9]+]] = reduction-final{s_smin} i32 [[TMP_EXIT]] i32 [[BEST_EXIT]] i32 [[BEST_FINAL]]
;CHECK-NEXT:  i32 {{%vp[0-9]+}} = reduction-final{s_smin} i32 [[VAL_EXIT]] i32 [[BEST_EXIT]] i32 [[BEST_FINAL]]

;DISABLED: Idiom List
;DISABLED-NEXT  No idioms detected.
;
; source code
;int ordering[1000];
;int  maxloc (int m) {
;    int best = -111111111;
;    int tmp = 0;
;    int val = 0;
;    for (int i=0; i< m; i++) {
;        if (ordering[i] > best) {
;            best = ordering[i];
;            tmp = i;
;            val = ordering[i]+2;
;        }
;    }
;    return tmp + best+val;
;}
;
; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @maxloc(i32 %m, i32* nocapture readonly %ordering) local_unnamed_addr #0 {
entry:
  %cmp22 = icmp sgt i32 %m, 0
  br i1 %cmp22, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
;  %wide.trip.count = sext i32 %m to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i32 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %val.025 = phi i32 [ 0, %for.body.preheader ], [ %spec.select21, %for.body ]
  %tmp.024 = phi i32 [ 0, %for.body.preheader ], [ %spec.select20, %for.body ]
  %best.023 = phi i32 [ -111111111, %for.body.preheader ], [ %spec.select, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ordering, i32 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, %best.023
  %add = add nsw i32 %0, 2
  %spec.select = select i1 %cmp1, i32 %0, i32 %best.023
  %spec.select20 = select i1 %cmp1, i32 %indvars.iv, i32 %tmp.024
  %spec.select21 = select i1 %cmp1, i32 %add, i32 %val.025
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %m
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body ]
  %spec.select20.lcssa = phi i32 [ %spec.select20, %for.body ]
  %spec.select21.lcssa = phi i32 [ %spec.select21, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %best.0.lcssa = phi i32 [ -111111111, %entry ], [ %spec.select.lcssa, %for.end.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select20.lcssa, %for.end.loopexit ]
  %val.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select21.lcssa, %for.end.loopexit ]
  %add6 = add nsw i32 %tmp.0.lcssa, %best.0.lcssa
  %add7 = add nsw i32 %add6, %val.0.lcssa
  ret i32 %add6
};
attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

