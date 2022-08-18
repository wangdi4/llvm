; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-entities-dump -disable-vplan-codegen -vplan-enable-masked-variant=false -vplan-vec-scenario="n0;s1;n0" 2>&1 | FileCheck %s

; Reduced lammps2021/sw code related to compress/expand idiom recognition.

; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP> <ivdep>
;       |   if (%cmp348 != 0)
;       |   {
;       |      if (undef true undef)
;       |      {
;       |         if (undef false undef)
;       |         {
;       |            goto if.end396;
;       |         }
;       |      }
;       |      else
;       |      {
;       |         if (undef true undef)
;       |         {
;       |            goto if.end396;
;       |         }
;       |      }
;       |   }
;       |   else
;       |   {
;       |      if (undef false undef)
;       |      {
;       |         goto if.end396;
;       |      }
;       |   }
;       |   if (undef true undef)
;       |   {
;       |      (%intel_list.0.val)[sext.i32.i64(%0) + sext.i32.i64(%n.priv.0292)] = %0;
;       |      %n.priv.0292 = %n.priv.0292  +  1;
;       |   }
;       |   if.end396:
;       + END LOOP
;
;       %n.priv.2296 = %n.priv.0292;
;
;       + DO i1 = 0, 1023, 1   <DO_LOOP> <ivdep>
;       |   if (undef false undef)
;       |   {
;       |      (%intel_list.0.val)[sext.i32.i64(%0) + sext.i32.i64(%n.priv.2296)] = (%ncachej.0.val)[i1 + sext.i32.i64((trunc.i64.i32(%maxnbors.0.val) * %0))];
;       |      %n.priv.2296 = %n.priv.2296  +  1;
;       |   }
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[N_PRIV_02920:%.*]] = [[N_PRIV_02920]]  +  1
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[N_PRIV_02920]] = [[N_PRIV_02920]]  +  1
; CHECK-NEXT:    CEStore: {{.*}} ([[INTEL_LIST_0_VAL0:%.*]])[sext.i32.i64([[TMP0:%.*]]) + sext.i32.i64([[N_PRIV_02920]])] = [[TMP0]]
; CHECK-NEXT:      CELdStIndex: sext.i32.i64([[TMP0]]) + sext.i32.i64([[N_PRIV_02920]])

; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP10:%.*]] = add i64 [[VP11:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP11]], i64 [[VP10]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP12:%.*]] = phi  [ i32 [[N_PRIV_02920]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP13:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[N_PRIV_02920]]
; CHECK-NEXT:    LiveOut: i32 [[VP13]] = phi  [ i32 [[VP12]], [[BB3:BB[0-9]+]] ],  [ i32 [[VP14:%.*]], [[BB4:BB[0-9]+]] ],  [ i32 [[VP12]], [[BB5:BB[0-9]+]] ],  [ i32 [[VP12]], [[BB6:BB[0-9]+]] ],  [ i32 [[VP12]], [[BB7:BB[0-9]+]] ]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP14]] = add i32 [[VP12]] i32 1
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store i32 [[TMP0]] i32* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP15:%.*]] = add i64 [[VP0:%.*]] i64 [[VP16:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP12]], i32 [[N_PRIV_02920]], i32 [[VP13]], i32 [[VP14]], void [[VP_STORE:%.*]], i64 [[VP15]],

; CHECK:       [Compress/Expand Idiom] Increment {sb:4}+1 detected: {{.*}} [[N_PRIV_22960:%.*]] = [[N_PRIV_22960]]  +  1
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:  CEIndexIncFirst: {{.*}} [[N_PRIV_22960]] = [[N_PRIV_22960]]  +  1
; CHECK-NEXT:    CEStore: {{.*}} ([[INTEL_LIST_0_VAL0]])[sext.i32.i64([[TMP0]]) + sext.i32.i64([[N_PRIV_22960]])] = ([[NCACHEJ_0_VAL0:%.*]])[i1 + sext.i32.i64((trunc.i64.i32([[MAXNBORS_0_VAL0:%.*]]) * [[TMP0]]))]
; CHECK-NEXT:      CELdStIndex: sext.i32.i64([[TMP0]]) + sext.i32.i64([[N_PRIV_22960]])

; CHECK:       Loop Entities of the loop with header [[BB15:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 1023 BinOp: i64 [[VP28:%.*]] = add i64 [[VP29:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP29]], i64 [[VP28]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP12:%.*]] = phi  [ i32 [[N_PRIV_22960]], [[BB16:BB[0-9]+]] ],  [ i32 [[VP11:%.*]], [[BB17:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[N_PRIV_22960]]
; CHECK-NEXT:    LiveOut: i32 [[VP11]] = phi  [ i32 [[VP19:%.*]], [[BB18:BB[0-9]+]] ],  [ i32 [[VP12]], [[BB15]] ]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP19]] = add i32 [[VP12]] i32 1
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store i32 [[VP_LOAD:%.*]] i32* [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP10:%.*]] = add i64 [[VP22:%.*]] i64 [[VP16:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP12]], i32 [[N_PRIV_22960]], i32 [[VP11]], i32 [[VP19]], void [[VP_STORE_1:%.*]], i64 [[VP10]],

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %intel_list.0.val, i64 %maxnbors.0.val, i32* %ncachej.0.val, i32 %0, i1 %cmp348) {
DIR.OMP.PARALLEL.2:
  %1 = trunc i64 %maxnbors.0.val to i32
  %mul133 = mul nsw i32 %0, %1
  %idx.ext134 = sext i32 %mul133 to i64
  %add.ptr141 = getelementptr inbounds i32, i32* %ncachej.0.val, i64 %idx.ext134
  br label %for.body149

for.body149:                                      ; preds = %DIR.OMP.PARALLEL.2
  %idx.ext.pn310.pn = sext i32 %0 to i64
  %neighptr.priv.0323 = getelementptr inbounds i32, i32* %intel_list.0.val, i64 %idx.ext.pn310.pn
  br label %for.body322.preheader

for.body322.preheader:                            ; preds = %for.body149
  br label %for.body322

for.cond402.preheader.loopexit:                   ; preds = %if.end396
  %n.priv.1.lcssa = phi i32 [ %n.priv.1, %if.end396 ]
  br label %for.body405.preheader

for.body405.preheader:                            ; preds = %for.cond402.preheader.loopexit
  br label %for.body405

for.body322:                                      ; preds = %if.end396, %for.body322.preheader
  %indvars.iv = phi i64 [ 0, %for.body322.preheader ], [ %indvars.iv.next, %if.end396 ]
  %n.priv.0292 = phi i32 [ 0, %for.body322.preheader ], [ %n.priv.1, %if.end396 ]
  br i1 %cmp348, label %if.then349, label %if.else366

if.then349:                                       ; preds = %for.body322
  br i1 true, label %if.then353, label %if.else357

if.then353:                                       ; preds = %if.then349
  br i1 false, label %if.end396, label %if.end390

if.else357:                                       ; preds = %if.then349
  br i1 false, label %if.end390, label %if.end396

if.else366:                                       ; preds = %for.body322
  br i1 false, label %if.then375, label %if.end390

if.then375:                                       ; preds = %if.else366
  br i1 false, label %if.end396, label %if.end390

if.end390:                                        ; preds = %if.then375, %if.else366, %if.else357, %if.then353
  br i1 false, label %if.end396, label %if.then392

if.then392:                                       ; preds = %if.end390
  %inc393 = add nsw i32 %n.priv.0292, 1
  %idxprom394 = sext i32 %n.priv.0292 to i64
  %arrayidx395 = getelementptr inbounds i32, i32* %neighptr.priv.0323, i64 %idxprom394
  store i32 %0, i32* %arrayidx395, align 4
  br label %if.end396

if.end396:                                        ; preds = %if.then392, %if.end390, %if.then375, %if.else357, %if.then353
  %n.priv.1 = phi i32 [ %n.priv.0292, %if.end390 ], [ %inc393, %if.then392 ], [ %n.priv.0292, %if.else357 ], [ %n.priv.0292, %if.then353 ], [ %n.priv.0292, %if.then375 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond340.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond340.not, label %for.cond402.preheader.loopexit, label %for.body322, !llvm.loop !0

for.cond.cleanup404.loopexit:                     ; preds = %if.end442
  %n.priv.3.lcssa = phi i32 [ %n.priv.3, %if.end442 ]
  br label %for.cond.cleanup404

for.cond.cleanup404:                              ; preds = %for.cond.cleanup404.loopexit
  br label %if.then448

for.body405:                                      ; preds = %if.end442, %for.body405.preheader
  %indvars.iv341 = phi i64 [ 0, %for.body405.preheader ], [ %indvars.iv.next342, %if.end442 ]
  %n.priv.2296 = phi i32 [ %n.priv.1.lcssa, %for.body405.preheader ], [ %n.priv.3, %if.end442 ]
  br i1 true, label %if.end442, label %if.then438

if.then438:                                       ; preds = %for.body405
  %arrayidx409 = getelementptr inbounds i32, i32* %add.ptr141, i64 %indvars.iv341
  %2 = load i32, i32* %arrayidx409, align 4
  %inc439 = add nsw i32 %n.priv.2296, 1
  %idxprom440 = sext i32 %n.priv.2296 to i64
  %arrayidx441 = getelementptr inbounds i32, i32* %neighptr.priv.0323, i64 %idxprom440
  store i32 %2, i32* %arrayidx441, align 4
  br label %if.end442

if.end442:                                        ; preds = %if.then438, %for.body405
  %n.priv.3 = phi i32 [ %inc439, %if.then438 ], [ %n.priv.2296, %for.body405 ]
  %indvars.iv.next342 = add nuw nsw i64 %indvars.iv341, 1
  %exitcond344.not = icmp eq i64 %indvars.iv.next342, 1024
  br i1 %exitcond344.not, label %for.cond.cleanup404.loopexit, label %for.body405, !llvm.loop !4

if.then448:                                       ; preds = %for.cond.cleanup404
  ret void
}

!0 = distinct !{!0, !1, !2, !3}
!1 = !{!"llvm.loop.mustprogress"}
!2 = !{!"llvm.loop.vectorize.ivdep_back"}
!3 = !{!"llvm.loop.intel.vector.aligned"}
!4 = distinct !{!4, !1, !2, !3}
