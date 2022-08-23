; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=hir-vplan-vec 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   if ((%C)[i1] != 0)
;       |   {
;       |      %mul = (%B)[%j.015]  *  3.000000e+00;
;       |      %add = %mul  +  2.000000e+00;
;       |      (%B)[%j.015] = %add;
;       |      %j.015 = %j.015  +  1;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: <16>         [[J_0150:%.*]] = [[J_0150]]  +  1
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK-NEXT: Idiom List
; CHECK-NEXT: CEIndexIncFirst: <16>         [[J_0150]] = [[J_0150]]  +  1
; CHECK-NEXT:   CELoad: ([[B0:%.*]])[%j.015]
; CHECK-NEXT:     CELdStIndex: [[J_0150]]
; CHECK-NEXT:   CEStore: <15>         ([[B0]])[%j.015] = [[ADD0:%.*]]
; CHECK-NEXT:     CELdStIndex: [[J_0150]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP4:%.*]] = add i64 [[VP5:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP5]], i64 [[VP4]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP6:%.*]] = phi  [ i32 [[J_0150]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP7:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[J_0150]]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP8:%.*]] = add i32 [[VP6]] i32 1
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP9:%.*]] double* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      double [[VP_LOAD:%.*]] = load double* [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP10:%.*]] = sext i32 [[VP6]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP6]], i32 [[J_0150]], i32 [[VP8]], void [[VP_STORE:%.*]], double [[VP_LOAD]], i64 [[VP10]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP16:%.*]] = compress-expand-index-init i32 [[J_0150]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP6]] = phi  [ i32 [[VP16]], [[BB1]] ],  [ i32 [[VP18:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP5]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP4]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP5]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP13]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[VP10]] = sext i32 [[VP6]] to i64
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_1]] = subscript inbounds double* [[B0]] i64 [[VP10]]
; CHECK-NEXT:       double [[VP17:%.*]] = expand-load double* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       double [[VP14:%.*]] = fmul double [[VP17]] double 3.000000e+00
; CHECK-NEXT:       double [[VP9]] = fadd double [[VP14]] double 2.000000e+00
; CHECK-NEXT:       double* [[VP_SUBSCRIPT]] = subscript inbounds double* [[B0]] i64 [[VP10]]
; CHECK-NEXT:       compress-store double [[VP9]] double* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP8]] = add i32 [[VP6]] i32 1
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP7]] = phi  [ i32 [[VP8]], [[BB4]] ],  [ i32 [[VP6]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP18]] = compress-expand-index-inc i32 [[VP7]]
; CHECK-NEXT:     i64 [[VP4]] = add i64 [[VP5]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP15:%.*]] = icmp slt i64 [[VP4]] i64 1024
; CHECK-NEXT:     br i1 [[VP15]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP19:%.*]] = compress-expand-index-final i32 [[VP18]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[INSERT0:%.*]] = insertelement zeroinitializer,  [[J_0150]],  0
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[INSERT0]]
; CHECK:             + DO i1 = 0, 1023, 32   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<32 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC10:%.*]] = [[DOTVEC0]] != 0
; CHECK-NEXT:        |   [[EXTRACT_0_0:%.*]] = extractelement &((<32 x double*>)([[B0]])[%phi.temp]),  0
; CHECK-NEXT:        |   [[EXP_LOAD0:%.*]] = @llvm.masked.expandload.v32f64([[EXTRACT_0_0]],  [[DOTVEC10]],  undef)
; CHECK-NEXT:        |   [[DOTVEC20:%.*]] = [[EXP_LOAD0]]  *  3.000000e+00
; CHECK-NEXT:        |   [[DOTVEC30:%.*]] = [[DOTVEC20]]  +  2.000000e+00
; CHECK-NEXT:        |   [[EXTRACT_0_40:%.*]] = extractelement &((<32 x double*>)([[B0]])[%phi.temp]),  0
; CHECK-NEXT:        |   @llvm.masked.compressstore.v32f64([[DOTVEC30]],  [[EXTRACT_0_40]],  [[DOTVEC10]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC10]] == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[PHI_TEMP0]] + 1 : [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.add.v32i32([[SELECT0]])
; CHECK-NEXT:        |   [[INSERT50:%.*]] = insertelement zeroinitializer,  [[VEC_REDUCE0]],  0
; CHECK-NEXT:        |   [[PHI_TEMP0]] = [[INSERT50]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[EXTRACT_0_70:%.*]] = extractelement [[INSERT50]],  0
; CHECK-NEXT:        [[J_0150]] = [[EXTRACT_0_70]]
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_Pii(double* noalias nocapture noundef readnone %A, double* noalias nocapture noundef %B, i32* noalias nocapture noundef readonly %C, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 1024, 0
  br i1 %cmp14, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 1024 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j.015 = phi i32 [ 0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom2 = sext i32 %j.015 to i64
  %arrayidx3 = getelementptr inbounds double, double* %B, i64 %idxprom2
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %mul = fmul fast double %1, 3.000000e+00
  %add = fadd fast double %mul, 2.000000e+00
  store double %add, double* %arrayidx3, align 8, !tbaa !7
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
