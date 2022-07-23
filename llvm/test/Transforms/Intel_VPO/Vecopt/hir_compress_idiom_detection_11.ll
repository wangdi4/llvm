; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-vplan-codegen -vplan-enable-masked-variant=false 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%C)[i1] != 0)
;       |   {
;       |      (%B)[%j.015] = (%A)[i1];
;       |      %j.015 = %j.015  +  1;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[J_0150:%.*]] = [[J_0150]]  +  1
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK-NEXT: Idiom List
; CHECK-NEXT: CEIndexIncFirst: {{.*}} [[J_0150]] = [[J_0150]]  +  1
; CHECK-NEXT:   CEStore: {{.*}} ([[B0:%.*]])[%j.015] = ([[A0:%.*]])[i1]
; CHECK-NEXT:     CELdStIndex: [[J_0150]]

; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: ? BinOp: i64 [[VP5:%.*]] = add i64 [[VP6:%.*]] i64 [[VP__IND_INIT_STEP:%.*]]
; CHECK-NEXT:    Linked values: i64 [[VP6]], i64 [[VP5]], i64 [[VP__IND_INIT:%.*]], i64 [[VP__IND_FINAL:%.*]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP8:%.*]] = phi  [ i32 [[J_0150]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP9:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[J_0150]]
; CHECK-NEXT:    LiveOut: i32 [[VP9]] = phi  [ i32 [[VP7:%.*]], [[BB3:BB[0-9]+]] ],  [ i32 [[VP8]], [[BB0]] ]
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP7]] = add i32 [[VP8]] i32 1 (Stride = 1)
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD:%.*]] double* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP10:%.*]] = sext i32 [[VP8]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP8]], i32 [[J_0150]], i32 [[VP9]], i32 [[VP7]], void [[VP_STORE:%.*]], i64 [[VP10]],
; CHECK:         [[BB4:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB4]]
; CHECK-NEXT:     i64 [[VP11:%.*]] = add i64 [[VP2:%.*]] i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP]] = induction-init-step{add} i64 1
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP8]] = phi  [ i32 [[J_0150]], [[BB1]] ],  [ i32 [[VP9]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP6]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP5]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP6]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP12:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP12]], [[BB3]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]]: # preds: [[BB0]]
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds double* [[A0]] i64 [[VP6]]
; CHECK-NEXT:       double [[VP_LOAD]] = load double* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       i64 [[VP10]] = sext i32 [[VP8]] to i64
; CHECK-NEXT:       double* [[VP_SUBSCRIPT]] = subscript inbounds double* [[B0]] i64 [[VP10]]
; CHECK-NEXT:       store double [[VP_LOAD]] double* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP7]] = add i32 [[VP8]] i32 1
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB3]], [[BB0]]
; CHECK-NEXT:     i32 [[VP9]] = phi  [ i32 [[VP7]], [[BB3]] ],  [ i32 [[VP8]], [[BB0]] ]
; CHECK-NEXT:     i64 [[VP5]] = add i64 [[VP6]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp slt i64 [[VP5]] i64 [[VP11]]
; CHECK-NEXT:     br i1 [[VP13]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>
; CHECK-EMPTY:
; CHECK-NEXT:  External Uses:
; CHECK-NEXT:  Id: 0   i32 [[VP9]] -> [[VP14:%.*]] = {%j.015}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooPdS_Piii(double* noalias nocapture noundef readonly %A, double* noalias nocapture noundef writeonly %B, i32* noalias nocapture noundef readonly %C, i32 noundef %N, i32 noundef %j0) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %N, 0
  br i1 %cmp14, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %j.1.lcssa = phi i32 [ %j.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %j.0.lcssa = phi i32 [ %j0, %entry ], [ %j.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %j.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j.015 = phi i32 [ %j0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, double* %A, i64 %indvars.iv
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %idxprom4 = sext i32 %j.015 to i64
  %arrayidx5 = getelementptr inbounds double, double* %B, i64 %idxprom4
  store double %1, double* %arrayidx5, align 8, !tbaa !7
  %inc = add nsw i32 %j.015, 1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %j.1 = phi i32 [ %inc, %if.then ], [ %j.015, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
