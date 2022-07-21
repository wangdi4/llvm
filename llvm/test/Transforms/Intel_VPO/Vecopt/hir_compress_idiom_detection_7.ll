; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-vplan-codegen 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <23>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <5>                |   if ((%a)[i1] != 0)
; <5>                |   {
; <9>                |      %k.034 = %k.034  +  3;
; <14>               |      (%v2)[i1] = (%d2)[%k.034];
; <5>                |   }
; <23>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+3 detected: <9>          [[K_0340:%.*]] = [[K_0340]]  +  3
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK:      Idiom List
; CHECK-NEXT: CEIndexIncFirst: <9>          [[K_0340]] = [[K_0340]]  +  3
; CHECK-NEXT:   CELoad: ([[D20:%.*]])[%k.034]
; CHECK-NEXT:     CELdStIndex: [[K_0340]]

; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: ? BinOp: i64 [[VP5:%.*]] = add i64 [[VP6:%.*]] i64 [[VP__IND_INIT_STEP:%.*]]
; CHECK-NEXT:    Linked values: i64 [[VP6]], i64 [[VP5]], i64 [[VP__IND_INIT:%.*]], i64 [[VP__IND_FINAL:%.*]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP7:%.*]] = add i32 [[VP8:%.*]] i32 3
; CHECK-NEXT:        Stride: 3
; CHECK-NEXT:        Init: i32 [[K_0340]]
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      i32* [[VP_SUBSCRIPT:%.*]] = subscript inbounds i32* [[D20]] i64 [[VP9:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i32 [[VP7]] = add i32 [[VP8]] i32 3
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP7]], i32 [[VP8]], i32 [[K_0340]], i32* [[VP_SUBSCRIPT]],

; CHECK:         [[BB1:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB2:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB1]]
; CHECK-NEXT:     i64 [[VP10:%.*]] = add i64 [[VP2:%.*]] i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP]] = induction-init-step{add} i64 1
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB2]], [[BB3:BB[0-9]+]]
; CHECK-NEXT:     i32 [[VP8]] = phi  [ i32 [[K_0340]], [[BB2]] ],  [ i32 [[VP11:%.*]], [[BB3]] ]
; CHECK-NEXT:     i64 [[VP6]] = phi  [ i64 [[VP__IND_INIT]], [[BB2]] ],  [ i64 [[VP5]], [[BB3]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds i32* [[A0:%.*]] i64 [[VP6]]
; CHECK-NEXT:     i32 [[VP_LOAD:%.*]] = load i32* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP12:%.*]] = icmp ne i32 [[VP_LOAD]] i32 0
; CHECK-NEXT:     br i1 [[VP12]], [[BB4:BB[0-9]+]], [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       i32 [[VP7]] = add i32 [[VP8]] i32 3
; CHECK-NEXT:       i64 [[VP9]] = sext i32 [[VP7]] to i64
; CHECK-NEXT:       i32* [[VP_SUBSCRIPT]] = subscript inbounds i32* [[D20]] i64 [[VP9]]
; CHECK-NEXT:       i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds i32* [[V20:%.*]] i64 [[VP6]]
; CHECK-NEXT:       store i32 [[VP_LOAD_1]] i32* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:       br [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB3]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP11]] = phi  [ i32 [[VP7]], [[BB4]] ],  [ i32 [[VP8]], [[BB0]] ]
; CHECK-NEXT:     i64 [[VP5]] = add i64 [[VP6]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP13:%.*]] = icmp slt i64 [[VP5]] i64 [[VP10]]
; CHECK-NEXT:     br i1 [[VP13]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPiS_S_S_S_S_S_i(i32* noalias nocapture noundef readonly %a, i32* noalias nocapture noundef writeonly %d1, i32* noalias nocapture noundef readonly %v1, i32* noalias nocapture noundef readonly %d2, i32* noalias nocapture noundef writeonly %v2, i32* noalias nocapture noundef writeonly %d3, i32* noalias nocapture noundef readonly %v3, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count36 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %k.034 = phi i32 [ 0, %for.body.preheader ], [ %k.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %k.034, 3
  %idxprom7 = sext i32 %add to i64
  %arrayidx8 = getelementptr inbounds i32, i32* %d2, i64 %idxprom7
  %ld2 = load i32, i32* %arrayidx8, align 4, !tbaa !3
  %arrayidx10 = getelementptr inbounds i32, i32* %v2, i64 %indvars.iv
  store i32 %ld2, i32* %arrayidx10, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %add, %if.then ], [ %k.034, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count36
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
