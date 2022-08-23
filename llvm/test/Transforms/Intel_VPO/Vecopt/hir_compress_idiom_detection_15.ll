; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=hir-vplan-vec -vplan-force-vf=16 2>&1 | FileCheck %s

; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=4 2>&1 | FileCheck %s --check-prefix=CM4
; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=8 2>&1 | FileCheck %s --check-prefix=CM8

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   if ((%c)[i1] != 0)
;       |   {
;       |      %add = (%a)[%k.013]  +  (%b)[%k.013];
;       |      (%a)[%k.013] = %add;
;       |      %k.013 = %k.013  +  5;
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+5 detected: {{.*}} [[K_0130:%.*]] = [[K_0130]]  +  5
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[K_0130]] = [[K_0130]]  +  5
; CHECK-NEXT:    CELoad: ([[A0:%.*]])[%k.013]
; CHECK-NEXT:      CELdStIndex: [[K_0130]]
; CHECK-NEXT:    CELoad: ([[B0:%.*]])[%k.013]
; CHECK-NEXT:      CELdStIndex: [[K_0130]]
; CHECK-NEXT:    CEStore: {{.*}} ([[A0]])[%k.013] = [[ADD0:%.*]]
; CHECK-NEXT:      CELdStIndex: [[K_0130]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP4:%.*]] = add i64 [[VP5:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP5]], i64 [[VP4]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP6:%.*]] = phi  [ i32 [[K_0130]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP7:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[K_0130]]
; CHECK-NEXT:    TotalStride: 5
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP8:%.*]] = add i32 [[VP6]] i32 5
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
; CHECK-NEXT:     i32 [[VP6]] = phi  [ i32 [[VP15]], [[BB1]] ],  [ i32 [[VP21:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP5]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP4]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP5]]
; CHECK-NEXT:     i32 [[VP_LOAD_2:%.*]] = load i32* [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp ne i32 [[VP_LOAD_2]] i32 0
; CHECK-NEXT:     br i1 [[VP13]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[VP10]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       i64 [[VP16:%.*]] = compress-expand-index i64 [[VP10]] i64 5
; CHECK-NEXT:       float* [[VP_SUBSCRIPT_1]] = subscript inbounds float* [[A0]] i64 [[VP16]]
; CHECK-NEXT:       float [[VP17:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       i64 [[VP11]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       i64 [[VP18:%.*]] = compress-expand-index i64 [[VP11]] i64 5
; CHECK-NEXT:       float* [[VP_SUBSCRIPT_2]] = subscript inbounds float* [[B0]] i64 [[VP18]]
; CHECK-NEXT:       float [[VP19:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       float [[VP9]] = fadd float [[VP17]] float [[VP19]]
; CHECK-NEXT:       i64 [[VP12]] = zext i32 [[VP6]] to i64
; CHECK-NEXT:       i64 [[VP20:%.*]] = compress-expand-index i64 [[VP12]] i64 5
; CHECK-NEXT:       float* [[VP_SUBSCRIPT]] = subscript inbounds float* [[A0]] i64 [[VP20]]
; CHECK-NEXT:       compress-store-nonu float [[VP9]] float* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP8]] = add i32 [[VP6]] i32 5
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP7]] = phi  [ i32 [[VP8]], [[BB4]] ],  [ i32 [[VP6]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP21]] = compress-expand-index-inc i32 [[VP7]]
; CHECK-NEXT:     i64 [[VP4]] = add i64 [[VP5]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP14:%.*]] = icmp slt i64 [[VP4]] i64 1024
; CHECK-NEXT:     br i1 [[VP14]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP22:%.*]] = compress-expand-index-final i32 [[VP21]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[INSERT0:%.*]] = insertelement zeroinitializer,  [[K_0130]],  0
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[INSERT0]]
; CHECK:             + DO i1 = 0, 1023, 16   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<16 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC10:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector [[PHI_TEMP0]],  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD20:%.*]] = [[SHUFFLE0]]  +  <i64 0, i64 5, i64 10, i64 15, i64 20, i64 25, i64 30, i64 35, i64 40, i64 45, i64 50, i64 55, i64 60, i64 65, i64 70, i64 75>
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC10]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i16([[CAST0]])
; CHECK-NEXT:        |   [[SHL0:%.*]] = -1  <<  [[POPCNT0]]
; CHECK-NEXT:        |   [[XOR0:%.*]] = [[SHL0]]  ^  -1
; CHECK-NEXT:        |   [[CAST30:%.*]] = bitcast.i16.<16 x i1>([[XOR0]])
; CHECK-NEXT:        |   [[GATHER0:%.*]] = @llvm.masked.gather.v16f32.v16p0f32(&((<16 x float*>)([[A0]])[%add2]),  0,  [[CAST30]],  undef)
; CHECK-NEXT:        |   [[EXPAND0:%.*]] = @llvm.x86.avx512.mask.expand.v16f32([[GATHER0]],  undef,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[SHUFFLE40:%.*]] = shufflevector [[PHI_TEMP0]],  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD60:%.*]] = [[SHUFFLE40]]  +  <i64 0, i64 5, i64 10, i64 15, i64 20, i64 25, i64 30, i64 35, i64 40, i64 45, i64 50, i64 55, i64 60, i64 65, i64 70, i64 75>
; CHECK-NEXT:        |   [[CAST70:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC10]])
; CHECK-NEXT:        |   [[POPCNT80:%.*]] = @llvm.ctpop.i16([[CAST70]])
; CHECK-NEXT:        |   [[SHL90:%.*]] = -1  <<  [[POPCNT80]]
; CHECK-NEXT:        |   [[XOR100:%.*]] = [[SHL90]]  ^  -1
; CHECK-NEXT:        |   [[CAST110:%.*]] = bitcast.i16.<16 x i1>([[XOR100]])
; CHECK-NEXT:        |   [[GATHER120:%.*]] = @llvm.masked.gather.v16f32.v16p0f32(&((<16 x float*>)([[B0]])[%add6]),  0,  [[CAST110]],  undef)
; CHECK-NEXT:        |   [[EXPAND130:%.*]] = @llvm.x86.avx512.mask.expand.v16f32([[GATHER120]],  undef,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[DOTVEC140:%.*]] = [[EXPAND0]]  +  [[EXPAND130]]
; CHECK-NEXT:        |   [[SHUFFLE150:%.*]] = shufflevector [[PHI_TEMP0]],  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD160:%.*]] = [[SHUFFLE150]]  +  <i64 0, i64 5, i64 10, i64 15, i64 20, i64 25, i64 30, i64 35, i64 40, i64 45, i64 50, i64 55, i64 60, i64 65, i64 70, i64 75>
; CHECK-NEXT:        |   [[COMPRESS0:%.*]] = @llvm.x86.avx512.mask.compress.v16f32([[DOTVEC140]],  undef,  [[DOTVEC10]])
; CHECK-NEXT:        |   [[CAST170:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC10]])
; CHECK-NEXT:        |   [[POPCNT180:%.*]] = @llvm.ctpop.i16([[CAST170]])
; CHECK-NEXT:        |   [[SHL190:%.*]] = -1  <<  [[POPCNT180]]
; CHECK-NEXT:        |   [[XOR200:%.*]] = [[SHL190]]  ^  -1
; CHECK-NEXT:        |   [[CAST210:%.*]] = bitcast.i16.<16 x i1>([[XOR200]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v16f32.v16p0f32([[COMPRESS0]],  &((<16 x float*>)([[A0]])[%add16]),  0,  [[CAST210]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC10]] == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[PHI_TEMP0]] + 5 : [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.add.v16i32([[SELECT0]])
; CHECK-NEXT:        |   [[INSERT220:%.*]] = insertelement zeroinitializer,  [[VEC_REDUCE0]],  0
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[INSERT220]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[EXTRACT_0_0:%.*]] = extractelement [[INSERT220]],  0
; CHECK-NEXT:        [[K_0130]] = [[EXTRACT_0_0]]
; CHECK-NEXT:  END REGION

; CM4: Cost 1 for i32 [[VP0:%.*]] = compress-expand-index-init i32 live-in1
; CM4: Cost 2 for i64 [[VP7:%.*]] = compress-expand-index i64 [[VP6:%.*]] i64 5
; CM4: Cost 15 for float [[VP8:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_1:%.*]]
; CM4: Cost 2 for i64 [[VP10:%.*]] = compress-expand-index i64 [[VP9:%.*]] i64 5
; CM4: Cost 15 for float [[VP11:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_2:%.*]]
; CM4: Cost 2 for i64 [[VP14:%.*]] = compress-expand-index i64 [[VP13:%.*]] i64 5
; CM4: Cost 16 for compress-store-nonu float [[VP12:%.*]] float* [[VP_SUBSCRIPT_3:%.*]]
; CM4: Cost 4 for i32 [[VP15:%.*]] = compress-expand-index-inc i32 [[VP1:%.*]]
; CM4: Cost Unknown for i32 [[VP17:%.*]] = compress-expand-index-final i32 [[VP__BLEND_BB4:%.*]]

; CM8: Cost 1 for i32 [[VP0:%.*]] = compress-expand-index-init i32 live-in1
; CM8: Cost 2 for i64 [[VP7:%.*]] = compress-expand-index i64 [[VP6:%.*]] i64 5
; CM8: Cost 16 for float [[VP8:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_1:%.*]]
; CM8: Cost 2 for i64 [[VP10:%.*]] = compress-expand-index i64 [[VP9:%.*]] i64 5
; CM8: Cost 16 for float [[VP11:%.*]] = expand-load-nonu float* [[VP_SUBSCRIPT_2:%.*]]
; CM8: Cost 2 for i64 [[VP14:%.*]] = compress-expand-index i64 [[VP13:%.*]] i64 5
; CM8: Cost 20 for compress-store-nonu float [[VP12:%.*]] float* [[VP_SUBSCRIPT_3:%.*]]
; CM8: Cost 6 for i32 [[VP15:%.*]] = compress-expand-index-inc i32 [[VP1:%.*]]
; CM8: Cost Unknown for i32 [[VP17:%.*]] = compress-expand-index-final i32 [[VP__BLEND_BB4:%.*]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z4vaddjPfS_Pi(i32 noundef %n, float* noalias nocapture noundef %a, float* noalias nocapture noundef readonly %b, i32* noalias nocapture noundef readonly %c) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %k.013 = phi i32 [ 0, %entry ], [ %k.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom1 = zext i32 %k.013 to i64
  %arrayidx2 = getelementptr inbounds float, float* %b, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !7
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %idxprom1
  %2 = load float, float* %arrayidx4, align 4, !tbaa !7
  %add = fadd fast float %2, %1
  store float %add, float* %arrayidx4, align 4, !tbaa !7
  %add5 = add i32 %k.013, 5
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %add5, %if.then ], [ %k.013, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !9

for.end:                                          ; preds = %for.inc
  ret void
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
!8 = !{!"float", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
