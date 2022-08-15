; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-vplan-codegen 2>&1 | FileCheck %s

; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=4 2>&1 | FileCheck %s --check-prefix=CM4
; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=8 2>&1 | FileCheck %s --check-prefix=CM8

; <0>          BEGIN REGION { }
; <28>               + DO i1 = 0, 14, 1   <DO_LOOP>
; <7>                |   if ((%c)[i1] != 0)
; <7>                |   {
; <16>               |      %add = (%a)[%k.013]  +  (%b)[%k.013];
; <17>               |      (%a)[%k.013] = %add;
; <18>               |      %k.013 = %k.013  +  1;
; <7>                |   }
; <28>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <18>         [[K_0130:%.*]] = [[K_0130]]  +  1
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK-NEXT: Idiom List
; CHECK-NEXT: CEIndexIncFirst: <18>         [[K_0130]] = [[K_0130]]  +  1
; CHECK-DAG:    CEStore: <17>         ([[A0:%.*]])[%k.013] = [[ADD0:%.*]]
; CHECK-DAG:      CELdStIndex: [[K_0130]]
; CHECK-DAG:    CELoad: ([[A0]])[%k.013]
; CHECK-DAG:      CELdStIndex: [[K_0130]]
; CHECK-DAG:    CELoad: ([[B0:%.*]])[%k.013]
; CHECK-DAG:      CELdStIndex: [[K_0130]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 14 BinOp: i64 [[VP4:%.*]] = add i64 [[VP5:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP5]], i64 [[VP4]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP6:%.*]] = phi  [ i32 [[K_0130]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP7:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[K_0130]]
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP8:%.*]] = add i32 [[VP6]] i32 1 (Stride = 1)
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store float [[VP9:%.*]] float* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      float [[VP_LOAD:%.*]] = load float* [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:      float [[VP_LOAD_1:%.*]] = load float* [[VP_SUBSCRIPT_2:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP10:%.*]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:      i64 [[VP11:%.*]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:      i64 [[VP12:%.*]] = zext i32 [[VP6]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP6]], i32 [[K_0130]], i32 [[VP8]], void [[VP_STORE:%.*]], float [[VP_LOAD]], float [[VP_LOAD_1]], i64 [[VP10]], i64 [[VP11]], i64 [[VP12]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP15:%.*]] = compress-expand-index-init i32 [[K_0130]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP6]] = phi  [ i32 [[VP15]], [[BB1]] ],  [ i32 [[VP7]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP5]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP4]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP5]]
; CHECK-NEXT:     i32 [[VP_LOAD_2:%.*]] = load i32* [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp ne i32 [[VP_LOAD_2]] i32 0
; CHECK-NEXT:     br i1 [[VP13]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[VP10]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       float* [[VP_SUBSCRIPT_1]] = subscript inbounds float* [[A0]] i64 [[VP10]]
; CHECK-NEXT:       float [[VP16:%.*]] = expand-load float* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       i64 [[VP11]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       float* [[VP_SUBSCRIPT_2]] = subscript inbounds float* [[B0]] i64 [[VP11]]
; CHECK-NEXT:       float [[VP17:%.*]] = expand-load float* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       float [[VP9]] = fadd float [[VP16]] float [[VP17]]
; CHECK-NEXT:       i64 [[VP12]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       float* [[VP_SUBSCRIPT]] = subscript inbounds float* [[A0]] i64 [[VP12]]
; CHECK-NEXT:       compress-store float [[VP9]] float* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP18:%.*]] = compress-expand-index-inc i32 [[VP6]] i32 1
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP7]] = phi  [ i32 [[VP18]], [[BB4]] ],  [ i32 [[VP6]], [[BB0]] ]
; CHECK-NEXT:     i64 [[VP4]] = add i64 [[VP5]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP14:%.*]] = icmp slt i64 [[VP4]] i64 15
; CHECK-NEXT:     br i1 [[VP14]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP19:%.*]] = compress-expand-index-final i32 [[VP7]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CM4: Cost 3 for i32 [[VP0:%.*]] = compress-expand-index-init i32 [[K_0130:%.*]]
; CM4: Cost 10 for float [[VP7:%.*]] = expand-load float* [[VP_SUBSCRIPT_1:%.*]]
; CM4: Cost 10 for float [[VP9:%.*]] = expand-load float* [[VP_SUBSCRIPT_2:%.*]]
; CM4: Cost 10 for compress-store float [[VP10:%.*]] float* [[VP_SUBSCRIPT_3:%.*]]
; CM4: Cost 6 for i32 [[VP12:%.*]] = compress-expand-index-inc i32 [[VP1:%.*]] i32 1
; CM4: Cost Unknown for i32 [[VP14:%.*]] = compress-expand-index-final i32 [[VP__BLEND_BB4:%.*]]

; CM8: Cost 3 for i32 [[VP0:%.*]] = compress-expand-index-init i32 [[K_0130:%.*]]
; CM8: Cost 20 for float [[VP7:%.*]] = expand-load float* [[VP_SUBSCRIPT_1:%.*]]
; CM8: Cost 20 for float [[VP9:%.*]] = expand-load float* [[VP_SUBSCRIPT_2:%.*]]
; CM8: Cost 20 for compress-store float [[VP10:%.*]] float* [[VP_SUBSCRIPT_3:%.*]]
; CM8: Cost 7 for i32 [[VP12:%.*]] = compress-expand-index-inc i32 [[VP1:%.*]] i32 1
; CM8: Cost Unknown for i32 [[VP14:%.*]] = compress-expand-index-final i32 [[VP__BLEND_BB4:%.*]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z4vaddjPfS_Pi(i32 noundef %n, float* noalias nocapture noundef %a, float* noalias nocapture noundef readonly %b, i32* noalias nocapture noundef readonly %c) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ], !in.de.ssa !3
  %k.013 = phi i32 [ 0, %entry ], [ %k.1, %for.inc ]
  %k.013.out = call i32 @llvm.ssa.copy.i32(i32 %k.013), !out.de.ssa !4
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !5
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom1 = zext i32 %k.013.out to i64
  %arrayidx2 = getelementptr inbounds float, float* %b, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !9
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %idxprom1
  %2 = load float, float* %arrayidx4, align 4, !tbaa !9
  %add = fadd fast float %2, %1
  store float %add, float* %arrayidx4, align 4, !tbaa !9
  %inc = add i32 %k.013, 1, !live.range.de.ssa !4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %inc, %if.then ], [ %k.013, %for.body ], !live.range.de.ssa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 15
  %indvars.iv.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next), !in.de.ssa !3
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !11

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #1

attributes #0 = { argmemonly mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone willreturn }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!"indvars.iv.de.ssa"}
!4 = !{}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
