; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -disable-output -debug-only=parvec-analysis -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -vplan-force-vf=4 -vplan-enable-masked-variant=false 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
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
; CHECK-NEXT:    CEStore: {{.*}} ([[B0:%.*]])[%j.022] = [[TMP1:%.*]];
; CHECK-NEXT:      CELdStIndex: [[J_0220]]
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[J2_0230]] = [[J2_0230]]  +  2
; CHECK-NEXT:    CEStore: {{.*}} ([[B20:%.*]])[%j2.023] = [[TMP1]]
; CHECK-NEXT:      CELdStIndex: [[J2_0230]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP7:%.*]] = add i64 [[VP8:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP8]], i64 [[VP7]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP9:%.*]] = phi  [ i32 [[J_0220]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP10:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[J_0220]]
; CHECK-NEXT:    TotalStride: 3
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP11:%.*]] = add i32 [[VP9]] i32 3
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD:%.*]] ptr [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP12:%.*]] = sext i32 [[VP9]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP9]], i32 [[J_0220]], i32 [[VP11]], void [[VP_STORE:%.*]], i64 [[VP12]],
; CHECK-EMPTY:
; CHECK-NEXT:    Phi: i32 [[VP13:%.*]] = phi  [ i32 [[J2_0230]], [[BB1]] ],  [ i32 [[VP14:%.*]], [[BB2]] ]
; CHECK-NEXT:    LiveIn: i32 [[J2_0230]]
; CHECK-NEXT:    TotalStride: 2
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP15:%.*]] = add i32 [[VP13]] i32 2
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD]] ptr [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP16:%.*]] = sext i32 [[VP13]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP13]], i32 [[J2_0230]], i32 [[VP15]], void [[VP_STORE_1:%.*]], i64 [[VP16]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP20:%.*]] = compress-expand-index-init i32 [[J_0220]]
; CHECK-NEXT:     i32 [[VP21:%.*]] = compress-expand-index-init i32 [[J2_0230]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP13]] = phi  [ i32 [[VP21]], [[BB1]] ],  [ i32 [[VP25:%.*]], [[BB2]] ]
; CHECK-NEXT:     i32 [[VP9]] = phi  [ i32 [[VP20]], [[BB1]] ],  [ i32 [[VP23:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP8]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP7]], [[BB2]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds ptr [[C0:%.*]] i64 [[VP8]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load ptr [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:     i1 [[VP18:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP18]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[MASK:%.*]] = compress-expand-mask
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds ptr [[A0:%.*]] i64 [[VP8]]
; CHECK-NEXT:       double [[VP_LOAD]] = load ptr [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       i64 [[VP12]] = sext i32 [[VP9]] to i64
; CHECK-NEXT:       i64 [[VP22:%.*]] = compress-expand-index {stride: 3} i64 [[VP12]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT]] = subscript inbounds ptr [[B0]] i64 [[VP22]]
; CHECK-NEXT:       compress-store-nonu double [[VP_LOAD]] ptr [[VP_SUBSCRIPT]] i64 [[MASK]]
; CHECK-NEXT:       i64 [[VP16]] = sext i32 [[VP13]] to i64
; CHECK-NEXT:       i64 [[VP24:%.*]] = compress-expand-index {stride: 2} i64 [[VP16]]
; CHECK-NEXT:       ptr [[VP_SUBSCRIPT_1]] = subscript inbounds ptr [[B20]] i64 [[VP24]]
; CHECK-NEXT:       compress-store-nonu double [[VP_LOAD]] ptr [[VP_SUBSCRIPT_1]] i64 [[MASK]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i1 [[VP14:%.*]] = phi  [ i1 true, [[BB4]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i1 [[VP10:%.*]] = phi  [ i1 true, [[BB4]] ],  [ i1 false, [[BB0]] ]
; CHECK-NEXT:     i32 [[VP25]] = compress-expand-index-inc {stride: 2} i32 [[VP13]] i1 [[VP14]]
; CHECK-NEXT:     i32 [[VP23]] = compress-expand-index-inc {stride: 3} i32 [[VP9]] i1 [[VP10]]
; CHECK-NEXT:     i64 [[VP7]] = add i64 [[VP8]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP19:%.*]] = icmp slt i64 [[VP7]] i64 1024
; CHECK-NEXT:     br i1 [[VP19]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP26:%.*]] = compress-expand-index-final i32 [[VP23]]
; CHECK-NEXT:     i32 [[VP27:%.*]] = compress-expand-index-final i32 [[VP25]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[J2_0230]]
; CHECK-NEXT:        [[PHI_TEMP10:%.*]] = [[J_0220]]
; CHECK:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC50:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC30:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[DOTVEC30]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[SHL0:%.*]] = -1  <<  [[POPCNT0]]
; CHECK-NEXT:        |   [[XOR0:%.*]] = [[SHL0]]  ^  -1
; CHECK-NEXT:        |   [[CAST40:%.*]] = bitcast.i4.<4 x i1>([[XOR0]])
; CHECK-NEXT:        |   [[DOTVEC50]] = (<4 x double>*)([[A0]])[i1], Mask = @{[[DOTVEC30]]}
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector [[PHI_TEMP10]],  poison,  zeroinitializer
; CHECK-NEXT:        |   [[ADD60:%.*]] = [[SHUFFLE0]]  +  <i64 0, i64 3, i64 6, i64 9>
; CHECK-NEXT:        |   [[COMPRESS0:%.*]] = @llvm.x86.avx512.mask.compress.v4f64([[DOTVEC50]],  poison,  [[DOTVEC30]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v4f64.v4p0([[COMPRESS0]],  &((<4 x ptr>)([[B0]])[%add6]),  0,  [[CAST40]])
; CHECK-NEXT:        |   [[SHUFFLE70:%.*]] = shufflevector [[PHI_TEMP0]],  poison,  zeroinitializer
; CHECK-NEXT:        |   [[ADD80:%.*]] = [[SHUFFLE70]]  +  <i64 0, i64 2, i64 4, i64 6>
; CHECK-NEXT:        |   [[COMPRESS90:%.*]] = @llvm.x86.avx512.mask.compress.v4f64([[DOTVEC50]],  poison,  [[DOTVEC30]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v4f64.v4p0([[COMPRESS90]],  &((<4 x ptr>)([[B20]])[%add8]),  0,  [[CAST40]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC30]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[SELECT100:%.*]] = ([[DOTVEC30]] == <i1 true, i1 true, i1 true, i1 true>) ? -1 : 0
; CHECK-NEXT:        |   [[CAST110:%.*]] = bitcast.<4 x i1>.i4([[SELECT0]])
; CHECK-NEXT:        |   [[POPCNT120:%.*]] = @llvm.ctpop.i4([[CAST110]])
; CHECK-NEXT:        |   [[ZEXT0:%.*]] = zext.i4.i32([[POPCNT120]])
; CHECK-NEXT:        |   [[MUL0:%.*]] = [[ZEXT0]]  *  2
; CHECK-NEXT:        |   [[ADD130:%.*]] = [[PHI_TEMP0]]  +  [[MUL0]]
; CHECK-NEXT:        |   [[CAST140:%.*]] = bitcast.<4 x i1>.i4([[SELECT100]])
; CHECK-NEXT:        |   [[POPCNT150:%.*]] = @llvm.ctpop.i4([[CAST140]])
; CHECK-NEXT:        |   [[ZEXT160:%.*]] = zext.i4.i32([[POPCNT150]])
; CHECK-NEXT:        |   [[MUL170:%.*]] = [[ZEXT160]]  *  3
; CHECK-NEXT:        |   [[ADD180:%.*]] = [[PHI_TEMP10]]  +  [[MUL170]]
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[ADD130]]
; CHECK-NEXT:        |   [[PHI_TEMP10]] = [[ADD180]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[J_0220]] = [[ADD180]]
; CHECK-NEXT:        [[J2_0230]] = [[ADD130]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_S_PiiiS0_S0_(ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef writeonly %B, ptr noalias nocapture noundef writeonly %B2, ptr noalias nocapture noundef readonly %C, i32 noundef %N, i32 noundef %j0, ptr noalias nocapture noundef readnone %out1, ptr noalias nocapture noundef readnone %out2) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 1024, 0
  br i1 %cmp21, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 1024 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j2.023 = phi i32 [ 0, %for.body.preheader ], [ %j2.1, %for.inc ]
  %j.022 = phi i32 [ 0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, ptr %A, i64 %indvars.iv
  %1 = load double, ptr %arrayidx3, align 8, !tbaa !7
  %idxprom4 = sext i32 %j.022 to i64
  %arrayidx5 = getelementptr inbounds double, ptr %B, i64 %idxprom4
  store double %1, ptr %arrayidx5, align 8, !tbaa !7
  %add = add nsw i32 %j.022, 3
  %idxprom8 = sext i32 %j2.023 to i64
  %arrayidx9 = getelementptr inbounds double, ptr %B2, i64 %idxprom8
  store double %1, ptr %arrayidx9, align 8, !tbaa !7
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
