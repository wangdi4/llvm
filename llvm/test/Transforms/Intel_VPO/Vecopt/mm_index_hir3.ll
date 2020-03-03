; Test for basic functionality of min/max+index idiom (main reduction + last linear index).
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -disable-vplan-codegen -enable-mmindex=1 -disable-nonlinear-mmindex=1 -vplan-print-after-vpentity-instrs -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -enable-vp-value-codegen-hir=1 -enable-mmindex=1 -disable-nonlinear-mmindex=1 -hir-cg -vplan-force-vf=4  -S -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck -check-prefix VPVAL_CG %s
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -vplan-plain-dump -vplan-entities-dump -enable-vp-value-codegen-hir=0 -enable-mmindex=1 -disable-nonlinear-mmindex=1 -hir-cg -vplan-force-vf=4  -S -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck -check-prefix MIXED_CG %s
;
;CHECK: Reduction list
;CHECK-NEXT: signed (SIntMax) Start: i32 [[BEST:%best.[0-9]+]] Exit: i32 [[BEST_EXIT:%vp[0-9]+]]
;CHECK-NEXT:   Linked values:{{.*}}
;CHECK-EMPTY:
;CHECK-NEXT: signed (SIntMax) Start: i32 [[TMP:%tmp.[0-9]+]] Exit: i32 [[TMP_EXIT:%vp[0-9]+]]
;CHECK-NEXT:   Linked values:{{.*}}
;CHECK-NEXT:  Parent exit: i32 [[BEST_EXIT]]
;CHECK-EMPTY:
;CHECK-NEXT: Induction list
;CHECK-NEXT: IntInduction(+) Start: i32 0 Step: i32 1 BinOp: i32 {{%vp[0-9]+}} = add i32 {{%vp[0-9]+}} i32 {{%vp[0-9]+}}
;
;CHECK:  BB3:
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = add
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = reduction-init i32 [[BEST]]
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = reduction-init i32 [[TMP]]
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = induction-init{add} i32 0 i32 1
;CHECK-NEXT:   i32 {{%vp[0-9]+}} = induction-init-step{add} i32 1
;
;CHECK:  BB5:
;CHECK-NEXT:  i32 [[BEST_FINAL:%vp[0-9]+]] = reduction-final{u_smax} i32 [[BEST_EXIT]]
;CHECK-NEXT:  i32 {{%vp[0-9]+}} = reduction-final{s_smax} i32 [[TMP_EXIT]] i32 [[BEST_EXIT]] i32 [[BEST_FINAL]]
;
;VPVAL_CG-LABEL:*** IR Dump After VPlan Vectorization Driver HIR ***
;VPVAL_CG: Function: maxloc
;VPVAL_CG-EMPTY:
;VPVAL_CG-NEXT:   BEGIN REGION { modified }
;VPVAL_CG-NEXT:         %tgu = (%m)/u4;
;VPVAL_CG-NEXT:         if (0 <u 4 * %tgu)
;VPVAL_CG-NEXT:         {
;VPVAL_CG-NEXT:            %red.var = %best.023;
;VPVAL_CG-NEXT:            %red.var1 = %tmp.024;
;VPVAL_CG:                 + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 1073741823> <nounroll> <novectorize>
;VPVAL_CG-NEXT:            |   %.vec = sext.<4 x i32>.<4 x i64>(i1 + <i32 0, i32 1, i32 2, i32 3>);
;VPVAL_CG-NEXT:            |   %.vec3 = (<4 x i32>*)(%ordering)[%.vec];
;VPVAL_CG-NEXT:            |   %.vec4 = %.vec3 >= %red.var;
;VPVAL_CG-NEXT:            |   %red.var1 = (%.vec3 >= %red.var) ? i1 + <i32 0, i32 1, i32 2, i32 3> : %red.var1;
;VPVAL_CG-NEXT:            |   %.vec6 = %.vec3 >= %red.var;
;VPVAL_CG-NEXT:            |   %red.var = (%.vec3 >= %red.var) ? %.vec3 : %red.var;
;VPVAL_CG-NEXT:            + END LOOP
;VPVAL_CG:                 %best.023 = @llvm.experimental.vector.reduce.smax.v4i32(%red.var);
;VPVAL_CG-NEXT:            %idx.blend = (%best.023 == %red.var) ? %red.var1 : <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>;
;VPVAL_CG-NEXT:            %tmp.024 = @llvm.experimental.vector.reduce.smax.v4i32(%idx.blend);
;VPVAL_CG-NEXT:         }
;VPVAL_CG:              + DO i1 = 4 * %tgu, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3> <nounroll> <novectorize> <max_trip_count = 3>
;VPVAL_CG-NEXT:         |   %0 = (%ordering)[i1];
;VPVAL_CG-NEXT:         |   %tmp.024 = (%0 >= %best.023) ? i1 : %tmp.024;
;VPVAL_CG-NEXT:         |   %best.023 = (%0 >= %best.023) ? %0 : %best.023;
;VPVAL_CG-NEXT:         + END LOOP
;VPVAL_CG-NEXT:   END REGION
;
;MIXED_CG-LABEL:*** IR Dump After VPlan Vectorization Driver HIR ***
;MIXED_CG: Function: maxloc
;MIXED_CG-EMPTY:
;MIXED_CG-NEXT:  BEGIN REGION { modified }
;MIXED_CG-NEXT:        %tgu = (%m)/u4;
;MIXED_CG-NEXT:        if (0 <u 4 * %tgu)
;MIXED_CG-NEXT:        {
;MIXED_CG-NEXT:           %red.var = %best.023;
;MIXED_CG-NEXT:           %red.var1 = %tmp.024;
;MIXED_CG:                + DO i1 = 0, 4 * %tgu + -1, 4   <DO_LOOP>  <MAX_TC_EST = 1073741823> <nounroll> <novectorize>
;MIXED_CG-NEXT:           |   %.vec = (<4 x i32>*)(%ordering)[i1];
;MIXED_CG-NEXT:           |   %.vec3 = %.vec >= %red.var;
;MIXED_CG-NEXT:           |   %red.var1 = (%.vec >= %red.var) ? i1 + <i32 0, i32 1, i32 2, i32 3> : %red.var1;
;MIXED_CG-NEXT:           |   %.vec5 = %.vec >= %red.var;
;MIXED_CG-NEXT:           |   %red.var = (%.vec >= %red.var) ? %.vec : %red.var;
;MIXED_CG-NEXT:           + END LOOP
;MIXED_CG:                %best.023 = @llvm.experimental.vector.reduce.smax.v4i32(%red.var);
;MIXED_CG-NEXT:           %idx.blend = (%best.023 == %red.var) ? %red.var1 : <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>;
;MIXED_CG-NEXT:           %tmp.024 = @llvm.experimental.vector.reduce.smax.v4i32(%idx.blend);
;MIXED_CG-NEXT:        }
;MIXED_CG:              + DO i1 = 4 * %tgu, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3> <nounroll> <novectorize> <max_trip_count = 3>
;MIXED_CG-NEXT:        |   %0 = (%ordering)[i1];
;MIXED_CG-NEXT:        |   %tmp.024 = (%0 >= %best.023) ? i1 : %tmp.024;
;MIXED_CG-NEXT:        |   %best.023 = (%0 >= %best.023) ? %0 : %best.023;
;MIXED_CG-NEXT:        + END LOOP
;MIXED_CG-NEXT:  END REGION
;
;int ordering[1000];
;int  maxloc (int m) {
;    int best = -111111111;
;    int tmp = 0;
;    int val = 0;
;    for (int i=0; i< m; i++) {
;        if (ordering[i] >= best) {
;            best = ordering[i];
;            tmp = i;
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
  %tmp.024 = phi i32 [ 0, %for.body.preheader ], [ %spec.select20, %for.body ]
  %best.023 = phi i32 [ -111111111, %for.body.preheader ], [ %spec.select, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ordering, i32 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sge i32 %0, %best.023
  %spec.select = select i1 %cmp1, i32 %0, i32 %best.023
;  %1 = trunc i64 %indvars.iv to i32
  %spec.select20 = select i1 %cmp1, i32 %indvars.iv, i32 %tmp.024
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, %m
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body ]
  %spec.select20.lcssa = phi i32 [ %spec.select20, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %best.0.lcssa = phi i32 [ -111111111, %entry ], [ %spec.select.lcssa, %for.end.loopexit ]
  %tmp.0.lcssa = phi i32 [ 0, %entry ], [ %spec.select20.lcssa, %for.end.loopexit ]
  %add6 = add nsw i32 %tmp.0.lcssa, %best.0.lcssa
  ret i32 %add6
};
attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

