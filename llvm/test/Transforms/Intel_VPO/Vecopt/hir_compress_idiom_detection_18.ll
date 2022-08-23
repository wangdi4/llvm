; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=hir-vplan-vec -vplan-force-vf=4 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   if ((%C)[i1] != 0)
;       |   {
;       |      (%B)[%j.013] = (%A)[i1];
;       |      %j.013 = %j.013  +  2;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+2 detected: {{.*}} [[J_0130:%.*]] = [[J_0130]]  +  2
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[J_0130]] = [[J_0130]]  +  2
; CHECK-NEXT:    CEStore: {{.*}} ([[B0:%.*]])[%j.013] = ([[A0:%.*]])[i1]
; CHECK-NEXT:      CELdStIndex: [[J_0130]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP5:%.*]] = add i64 [[VP6:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP6]], i64 [[VP5]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i64 [[VP7:%.*]] = phi  [ i64 [[J_0130]], [[BB1:BB[0-9]+]] ],  [ i64 [[VP8:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i64 [[J_0130]]
; CHECK-NEXT:    TotalStride: 2
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i64 [[VP9:%.*]] = add i64 [[VP7]] i64 2
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store i32 [[VP_LOAD:%.*]] i32* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP7]] = phi  [ i64 [[J_0130]], [[BB1]] ],  [ i64 [[VP8]], [[BB2]] ]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i64 [[VP7]], i64 [[J_0130]], i64 [[VP9]], void [[VP_STORE:%.*]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i64 [[VP13:%.*]] = compress-expand-index-init i64 [[J_0130]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i64 [[VP7]] = phi  [ i64 [[VP13]], [[BB1]] ],  [ i64 [[VP14:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP6]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP5]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP15:%.*]] = compress-expand-index i64 [[VP7]] i64 2
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP6]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP11:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP11]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i32* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds i32* [[A0]] i64 [[VP6]]
; CHECK-NEXT:       i32 [[VP_LOAD]] = load i32* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       i32* [[VP_SUBSCRIPT]] = subscript inbounds i32* [[B0]] i64 [[VP15]]
; CHECK-NEXT:       compress-store-nonu i32 [[VP_LOAD]] i32* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i64 [[VP9]] = add i64 [[VP7]] i64 2
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i64 [[VP8]] = phi  [ i64 [[VP9]], [[BB4]] ],  [ i64 [[VP7]], [[BB0]] ]
; CHECK-NEXT:     i64 [[VP14]] = compress-expand-index-inc i64 [[VP8]]
; CHECK-NEXT:     i64 [[VP5]] = add i64 [[VP6]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP12:%.*]] = icmp slt i64 [[VP5]] i64 1024
; CHECK-NEXT:     br i1 [[VP12]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP16:%.*]] = compress-expand-index-final i64 [[VP14]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[INSERT0:%.*]] = insertelement zeroinitializer,  [[J_0130]],  0
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[INSERT0]]
; CHECK:             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC20:%.*]] = undef
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector [[PHI_TEMP0]],  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD0:%.*]] = [[SHUFFLE0]]  +  <i64 0, i64 2, i64 4, i64 6>
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC10:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[DOTVEC20]] = (<4 x i32>*)([[A0]])[i1], Mask = @{[[DOTVEC10]]}
; CHECK-NEXT:        |   [[COMPRESS0:%.*]] = @llvm.x86.avx512.mask.compress.v4i32([[DOTVEC20]],  undef,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<4 x i1>.i4([[DOTVEC10]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i4([[CAST0]])
; CHECK-NEXT:        |   [[SHL0:%.*]] = -1  <<  [[POPCNT0]]
; CHECK-NEXT:        |   [[XOR0:%.*]] = [[SHL0]]  ^  -1
; CHECK-NEXT:        |   [[CAST30:%.*]] = bitcast.i4.<4 x i1>([[XOR0]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v4i32.v4p0i32([[COMPRESS0]],  &((<4 x i32*>)([[B0]])[%add]),  0,  [[CAST30]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC10]] == <i1 true, i1 true, i1 true, i1 true>) ? [[PHI_TEMP0]] + 2 : [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.add.v4i64([[SELECT0]])
; CHECK-NEXT:        |   [[INSERT40:%.*]] = insertelement zeroinitializer,  [[VEC_REDUCE0]],  0
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[INSERT40]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[EXTRACT_0_0:%.*]] = extractelement [[INSERT40]],  0
; CHECK-NEXT:        [[J_0130]] = [[EXTRACT_0_0]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree noinline norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPiS_S_i(i32* noalias nocapture noundef readonly %A, i32* noalias nocapture noundef writeonly %B, i32* noalias nocapture noundef readonly %C, i32 noundef %NN) local_unnamed_addr #0 {
entry:
  %conv = sext i32 1024 to i64
  %cmp12 = icmp sgt i32 1024, 0
  br i1 %cmp12, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.014 = phi i64 [ %inc5, %for.inc ], [ 0, %for.body.preheader ]
  %j.013 = phi i64 [ %j.1, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %i.014
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %i.014
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !3
  %arrayidx3 = getelementptr inbounds i32, i32* %B, i64 %j.013
  store i32 %1, i32* %arrayidx3, align 4, !tbaa !3
  %inc4 = add nsw i64 %j.013, 2
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %j.1 = phi i64 [ %inc4, %if.then ], [ %j.013, %for.body ]
  %inc5 = add nuw nsw i64 %i.014, 1
  %exitcond.not = icmp eq i64 %inc5, %conv
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { argmemonly mustprogress nofree noinline norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { mustprogress nofree norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly mustprogress nofree nounwind willreturn }
attributes #4 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #5 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #6 = { nofree nounwind }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!10, !4, i64 0}
!10 = !{!"array@_ZTSA16_i", !4, i64 0}
!11 = distinct !{!11, !8, !12}
!12 = !{!"llvm.loop.unroll.disable"}
