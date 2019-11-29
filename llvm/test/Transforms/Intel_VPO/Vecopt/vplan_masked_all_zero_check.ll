; RUN: opt -S -VPlanDriver -vplan-force-vf=8 < %s | FileCheck %s
; RUN: opt -S -VPlanDriver -vplan-force-vf=8 -enable-vp-value-codegen < %s | FileCheck %s

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i64 %N, i64* nocapture readonly %lb, i64* nocapture readonly %ub) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i64 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end9

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc7
  %i.022 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lb, i64 %i.022
  %0 = load i64, i64* %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %ub, i64 %i.022
  %1 = load i64, i64* %arrayidx2, align 8
  %cmp319 = icmp slt i64 %0, %1
; CHECK:  vector.body:
; CHECK:    [[WIDE_LOAD:%.*]] = load <8 x i64>, <8 x i64>* [[TMP1:%.*]], align 8
; CHECK:    [[WIDE_LOAD1:%.*]] = load <8 x i64>, <8 x i64>* [[TMP3:%.*]], align 8
; CHECK:    [[TMP4:%.*]] = icmp slt <8 x i64> [[WIDE_LOAD]], [[WIDE_LOAD1]]
  br i1 %cmp319, label %for.body4.preheader, label %for.inc7

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
; CHECK:       VPlannedBB:
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <8 x i64> [ [[TMP14:%.*]], [[VPLANNEDBB:%.*]] ], [ [[WIDE_LOAD]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI2:%.*]] = phi <8 x i1> [ [[TMP4]], [[VECTOR_BODY]] ], [ [[TMP13:%.*]], [[VPLANNEDBB]] ]
; CHECK-NEXT:    [[TMP5:%.*]] = and <8 x i1> [[TMP4]], [[VEC_PHI2]]
  %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ]
  %shl = shl i64 %j.020, 3
  %arrayidx6 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @A, i64 0, i64 %j.020, i64 %i.022
  store i64 %shl, i64* %arrayidx6, align 8
  %inc = add nsw i64 %j.020, 1
  %2 = load i64, i64* %arrayidx2, align 8
  %cmp3 = icmp slt i64 %inc, %2
; Verify that lanes masked out by [[TMP4]] don't take part in allzerocheck
; CHECK:         [[TMP15:%.*]] = and <8 x i1> [[TMP13]], [[TMP4]]
; CHECK-NEXT:    [[TMP16:%.*]] = bitcast <8 x i1> [[TMP15]] to i8
; CHECK-NEXT:    [[TMP17:%.*]] = icmp eq i8 [[TMP16]], 0
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT:%.*]] = insertelement <8 x i1> undef, i1 [[TMP17]], i32 0
; CHECK-NEXT:    [[BROADCAST_SPLAT:%.*]] = shufflevector <8 x i1> [[BROADCAST_SPLATINSERT]], <8 x i1> undef, <8 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP18:%.*]] = xor <8 x i1> [[BROADCAST_SPLAT]], <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
; CHECK-NEXT:    [[TMP19:%.*]] = extractelement <8 x i1> [[TMP18]], i32 0
; CHECK-NEXT:    br i1 [[TMP19]], label [[VPLANNEDBB]], label [[VPLANNEDBB3:%.*]]
  br i1 %cmp3, label %for.body4, label %for.inc7.loopexit

for.inc7.loopexit:                                ; preds = %for.body4
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc7.loopexit, %for.body
  %inc8 = add nuw nsw i64 %i.022, 1
  %exitcond = icmp eq i64 %inc8, %N
  br i1 %exitcond, label %for.end9.loopexit, label %for.body

for.end9.loopexit:                                ; preds = %for.inc7
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20869)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
