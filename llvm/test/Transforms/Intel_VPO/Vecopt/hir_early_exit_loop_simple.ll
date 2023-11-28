; Test to verify that HIRParVecAnalysis and VPlan HIR vectorizer can handle
; simple early exit loops.

; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -print-before=hir-vplan-vec -vplan-print-after-loop-massaging -vplan-enable-early-exit-loops -disable-output 2>&1 | FileCheck %s

; CHECK:  BEGIN REGION { }
; CHECK:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   %1 = (%a)[i1];
; CHECK:        |   if (%1 == %val)
; CHECK:        |   {
; CHECK:        |      goto cleanup.loopexit;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK:  END REGION

; CHECK-LABEL:  VPlan after loop massaging:
; CHECK-NEXT:  VPlan IR for: _Z3fooiPKaPaa:HIR.#{{[0-9]+}}
; CHECK-NEXT:  External Defs Start:
; CHECK-DAG:     [[VP0:%.*]] = {sext.i32.i64(%n) + -1}
; CHECK-DAG:     [[VP1:%.*]] = {%a}
; CHECK-DAG:     [[VP2:%.*]] = {%val}
; CHECK-NEXT:  External Defs End:
; CHECK-NEXT:    [[BB0:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB0]]
; CHECK-NEXT:     i64 [[VP3:%.*]] = add i64 [[VP0]] i64 1
; CHECK-NEXT:     i64 [[VP_VECTOR_TRIP_COUNT:%.*]] = vector-trip-count i64 [[VP3]], UF = 1
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 live-in0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     br [[BB2:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB1]], [[NEW_LOOP_LATCH0:new.loop.latch[0-9]+]]
; CHECK-NEXT:     i64 [[VP4:%.*]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP__SSA_PHI:%.*]], [[NEW_LOOP_LATCH0]] ]
; CHECK-NEXT:     ptr [[VP_SUBSCRIPT:%.*]] = subscript inbounds ptr [[A0:%.*]] i64 [[VP4]]
; CHECK-NEXT:     i8 [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT]]
; CHECK-NEXT:     i1 [[VP5:%.*]] = icmp eq i8 [[VP_LOAD]] i8 [[VAL0:%.*]]
; CHECK-NEXT:     i1 [[VP_EARLY_EXIT_COND:%.*]] = early-exit-cond i1 [[VP5]]
; CHECK-NEXT:     br i1 [[VP_EARLY_EXIT_COND]], [[INTERMEDIATE_BB0:intermediate.bb[0-9]+]], [[BB3:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]]: # preds: [[BB2]]
; CHECK-NEXT:       i64 [[VP6:%.*]] = add i64 [[VP4]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:       i1 [[VP7:%.*]] = icmp slt i64 [[VP6]] i64 [[VP_VECTOR_TRIP_COUNT]]
; CHECK-NEXT:       br [[NEW_LOOP_LATCH0]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[INTERMEDIATE_BB0]]: # preds: [[BB2]]
; CHECK-NEXT:       br [[NEW_LOOP_LATCH0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[NEW_LOOP_LATCH0]]: # preds: [[BB3]], [[INTERMEDIATE_BB0]]
; CHECK-NEXT:     i64 [[VP__SSA_PHI]] = phi  [ i64 [[VP6]], [[BB3]] ],  [ i64 [[VP4]], [[INTERMEDIATE_BB0]] ]
; CHECK-NEXT:     i32 [[VP_EXIT_ID_PHI:%.*]] = phi  [ i32 0, [[BB3]] ],  [ i32 1, [[INTERMEDIATE_BB0]] ]
; CHECK-NEXT:     i1 [[VP_TAKE_BACKEDGE_COND:%.*]] = phi  [ i1 [[VP7]], [[BB3]] ],  [ i1 false, [[INTERMEDIATE_BB0]] ]
; CHECK-NEXT:     br i1 [[VP_TAKE_BACKEDGE_COND]], [[BB2]], [[CASCADED_IF_BLOCK0:cascaded.if.block[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[CASCADED_IF_BLOCK0]]: # preds: [[NEW_LOOP_LATCH0]]
; CHECK-NEXT:     i1 [[VP8:%.*]] = icmp eq i32 [[VP_EXIT_ID_PHI]] i32 1
; CHECK-NEXT:     br i1 [[VP8]], [[BB4:BB[0-9]+]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB5]]: # preds: [[CASCADED_IF_BLOCK0]]
; CHECK-NEXT:       i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:       br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[CASCADED_IF_BLOCK0]]
; CHECK-NEXT:       br cleanup.loopexit
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB4]], [[BB5]]
; CHECK-NEXT:     br [[BB7:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB7]]: # preds: [[BB6]]
; CHECK-NEXT:     br <External Block>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z3fooiPKaPaa(i32 %n, ptr nocapture readonly %a, i8 signext %val) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  %1 = load i8, ptr %arrayidx, align 1
  %cmp2 = icmp eq i8 %1, %val
  br i1 %cmp2, label %cleanup.loopexit, label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %for.inc
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret i32 0
}
