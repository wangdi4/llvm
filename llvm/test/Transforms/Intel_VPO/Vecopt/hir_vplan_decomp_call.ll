; Test to check decomposition of function calls in incoming HIR.

; HIR incoming to vectorizer
; <0>     BEGIN REGION { }
; <22>          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <21>
; <21>          + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <3>           |   %0 = (%y)[i1];
; <9>           |   %call = @llvm.exp.f64(%0);
; <11>          |   (%x)[i1] = %call
; <5>           |   }
; <21>          + END LOOP
; <21>
; <23>          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>     END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -vplan-print-after-hcfg < %s 2>&1 | FileCheck %s

; CHECK-LABEL:  Print after building H-CFG:
; CHECK:          [DA: Divergent] i64 [[VP2:%.*]] = phi  [ i64 0, [[BB1:BB[0-9]+]] ],  [ i64 [[VP3:%.*]], [[BB3:BB[0-9]+]] ]
; CHECK-NEXT:     [DA: Divergent] double* [[VP4:%.*]] = getelementptr inbounds double* [[Y0:%.*]] i64 [[VP2]]
; CHECK-NEXT:     [DA: Divergent] double [[VP5:%.*]] = load double* [[VP4]]
; CHECK-NEXT:     [DA: Divergent] double [[VP6:%.*]] = call double [[VP5]] double (double)* @llvm.exp.f64
; CHECK-NEXT:     [DA: Divergent] double* [[VP7:%.*]] = getelementptr inbounds double* [[X0:%.*]] i64 [[VP2]]
; CHECK-NEXT:     [DA: Divergent] store double [[VP6]] double* [[VP7]]
; CHECK-NEXT:     [DA: Divergent] i64 [[VP3]] = add i64 [[VP2]] i64 1
; CHECK-NEXT:     [DA: Uniform]   i1 [[VP8:%.*]] = icmp i64 [[VP3]] i64 [[VP1:%vp.*]]

declare double @llvm.exp.f64(double %Val) nounwind readnone

define void @powi_f64(i32 %n, double* noalias nocapture readonly %y, double* noalias nocapture %x, i32 %P, double %key) local_unnamed_addr #2 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds double, double* %y, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %call = tail call fast double @llvm.exp.f64(double %0) #4
  %arrayidx4 = getelementptr inbounds double, double* %x, i64 %indvars.iv
  store double %call, double* %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

