; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-vplan-codegen -vplan-enable-masked-variant=false 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%C)[i1] != 0)
;       |   {
;       |      %1 = (%A)[i1];
;       |      (%B)[%j.022] = %1;
;       |      %j.022 = %j.022  +  3;
;       |      (%B2)[%j2.023] = %1;
;       |      %j2.023 = %j2.023  +  2;
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:4}+3 detected: {{.*}} [[J_0220:%.*]] = [[J_0220]]  +  3
; CHECK-NEXT:  [Compress/Expand Idiom] Increment {sb:3}+2 detected: {{.*}} [[J2_0230:%.*]] = [[J2_0230]]  +  2
; CHECK-NOT:   [Compress/Expand Idiom] Increment rejected
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[J_0220]] = [[J_0220]]  +  3
; CHECK-NEXT:    CEStore: {{.*}} ([[B0:%.*]])[%j.022] = [[TMP1:%.*]]
; CHECK-NEXT:      CELdStIndex: [[J_0220]]
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[J2_0230]] = [[J2_0230]]  +  2
; CHECK-NEXT:    CEStore: {{.*}} ([[B20:%.*]])[%j2.023] = [[TMP1]]
; CHECK-NEXT:      CELdStIndex: [[J2_0230]]

; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: ? BinOp: i64 [[VP7:%.*]] = add i64 [[VP8:%.*]] i64 [[VP__IND_INIT_STEP:%.*]]
; CHECK-NEXT:    Linked values: i64 [[VP8]], i64 [[VP7]], i64 [[VP__IND_INIT:%.*]], i64 [[VP__IND_FINAL:%.*]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP10:%.*]] = phi  [ i32 [[J_0220]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP11:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[J_0220]]
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP9:%.*]] = add i32 [[VP10]] i32 3 (Stride = 3)
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD:%.*]] double* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP12:%.*]] = sext i32 [[VP10]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP10]], i32 [[J_0220]], i32 [[VP9]], void [[VP_STORE:%.*]], i64 [[VP12]],
; CHECK-EMPTY:
; CHECK-NEXT:    Phi: i32 [[VP14:%.*]] = phi  [ i32 [[J2_0230]], [[BB1]] ],  [ i32 [[VP15:%.*]], [[BB2]] ]
; CHECK-NEXT:    LiveIn: i32 [[J2_0230]]
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP13:%.*]] = add i32 [[VP14]] i32 2 (Stride = 2)
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD]] double* [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP16:%.*]] = sext i32 [[VP14]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP14]], i32 [[J2_0230]], i32 [[VP13]], void [[VP_STORE_1:%.*]], i64 [[VP16]],
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP17:%.*]] = add i64 [[VP0:%.*]] i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP]] = induction-init-step{add} i64 1
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP14]] = phi  [ i32 [[J2_0230]], [[BB1]] ],  [ i32 [[VP15]], [[BB2]] ]
; CHECK-NEXT:     i32 [[VP10]] = phi  [ i32 [[J_0220]], [[BB1]] ],  [ i32 [[VP11]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP8]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP7]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP8]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:     i1 [[VP18:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP18]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds double* [[A0:%.*]] i64 [[VP8]]
; CHECK-NEXT:       double [[VP_LOAD]] = load double* [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       i64 [[VP12]] = sext i32 [[VP10]] to i64
; CHECK-NEXT:       double* [[VP_SUBSCRIPT]] = subscript inbounds double* [[B0]] i64 [[VP12]]
; CHECK-NEXT:       store double [[VP_LOAD]] double* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP9]] = add i32 [[VP10]] i32 3
; CHECK-NEXT:       i64 [[VP16]] = sext i32 [[VP14]] to i64
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_1]] = subscript inbounds double* [[B20]] i64 [[VP16]]
; CHECK-NEXT:       store double [[VP_LOAD]] double* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       i32 [[VP13]] = add i32 [[VP14]] i32 2
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP15]] = phi  [ i32 [[VP13]], [[BB4]] ],  [ i32 [[VP14]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP11]] = phi  [ i32 [[VP9]], [[BB4]] ],  [ i32 [[VP10]], [[BB0]] ]
; CHECK-NEXT:     i64 [[VP7]] = add i64 [[VP8]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP19:%.*]] = icmp slt i64 [[VP7]] i64 [[VP17]]
; CHECK-NEXT:     br i1 [[VP19]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_S_PiiiS0_S0_(double* noalias nocapture noundef readonly %A, double* noalias nocapture noundef writeonly %B, double* noalias nocapture noundef writeonly %B2, i32* noalias nocapture noundef readonly %C, i32 noundef %N, i32 noundef %j0, i32* noalias nocapture noundef readnone %out1, i32* noalias nocapture noundef readnone %out2) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j2.023 = phi i32 [ 0, %for.body.preheader ], [ %j2.1, %for.inc ]
  %j.022 = phi i32 [ 0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, double* %A, i64 %indvars.iv
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %idxprom4 = sext i32 %j.022 to i64
  %arrayidx5 = getelementptr inbounds double, double* %B, i64 %idxprom4
  store double %1, double* %arrayidx5, align 8, !tbaa !7
  %add = add nsw i32 %j.022, 3
  %idxprom8 = sext i32 %j2.023 to i64
  %arrayidx9 = getelementptr inbounds double, double* %B2, i64 %idxprom8
  store double %1, double* %arrayidx9, align 8, !tbaa !7
  %add10 = add nsw i32 %j2.023, 2
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %j.1 = phi i32 [ %add, %if.then ], [ %j.022, %for.body ]
  %j2.1 = phi i32 [ %add10, %if.then ], [ %j2.023, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
