; RUN: opt -S -passes=vplan-vec -vplan-enable-all-zero-bypass-loops=false -vplan-force-vf=8 < %s | FileCheck %s
; Temporarily disable all-zero bypass until CMPLRLLVM-21686 is resolved.

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i64 %N, ptr nocapture readonly %lb, ptr nocapture readonly %ub) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i64 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end9

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc7
  %i.022 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, ptr %lb, i64 %i.022
  %0 = load i64, ptr %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i64, ptr %ub, i64 %i.022
  %1 = load i64, ptr %arrayidx2, align 8
  %cmp319 = icmp slt i64 %0, %1
; CHECK:       vector.body:
; CHECK:         [[WIDE_LOAD0:%.*]] = load <8 x i64>, ptr [[TMP1:%.*]], align 8
; CHECK:         [[WIDE_LOAD50:%.*]] = load <8 x i64>, ptr [[TMP2:%.*]], align 8
; CHECK:         [[TMP3:%.*]] = icmp slt <8 x i64> [[WIDE_LOAD0]], [[WIDE_LOAD50]]
  br i1 %cmp319, label %for.body4.preheader, label %for.inc7

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
; CHECK:       VPlannedBB7:
; CHECK-NEXT:    [[VEC_PHI80:%.*]] = phi <8 x i64> [ [[TMP6:%.*]], [[VPLANNEDBB120:%.*]] ], [ [[WIDE_LOAD0]], [[VPLANNEDBB60:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI90:%.*]] = phi <8 x i1> [ [[TMP3]], [[VPLANNEDBB60]] ], [ [[TMP9:%.*]], [[VPLANNEDBB120:%.*]] ]
; CHECK-NEXT:    br label [[VPLANNEDBB100:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB10:
; CHECK-NEXT:    [[TMP4:%.*]] = select <8 x i1> [[TMP3]], <8 x i1> [[VEC_PHI90]], <8 x i1> zeroinitializer
  %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ]
  %shl = shl i64 %j.020, 3
  %arrayidx6 = getelementptr inbounds [100 x [100 x i64]], ptr @A, i64 0, i64 %j.020, i64 %i.022
  store i64 %shl, ptr %arrayidx6, align 8
  %inc = add nsw i64 %j.020, 1
  %2 = load i64, ptr %arrayidx2, align 8
  %cmp3 = icmp slt i64 %inc, %2
; Verify that lanes masked out by [[TMP4]] don't take part in allzerocheck
; CHECK:         [[TMP10:%.*]] = and <8 x i1> [[TMP9]], [[TMP3]]
; CHECK-NEXT:    [[TMP11:%.*]] = bitcast <8 x i1> [[TMP10]] to i8
; CHECK-NEXT:    [[TMP12:%.*]] = icmp eq i8 [[TMP11]], 0
; CHECK-NEXT:    br i1 [[TMP12]], label [[VPLANNEDBB130:%.*]], label [[VPLANNEDBB70:%.*]]
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
