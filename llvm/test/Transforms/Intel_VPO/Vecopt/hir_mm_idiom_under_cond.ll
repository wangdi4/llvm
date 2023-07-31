; RUN: opt -passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec -disable-output -vplan-entities-dump -vplan-print-after-vpentity-instrs %s 2>&1 | FileCheck %s

; Test to check that we don't crash on min/max+index idiom under condition.
; We don't vectorize the loop anyway because of a 'goto' presence.
;
;
; The incoming HIR is
; DO i1 = 0, 999, 1   <DO_LOOP>
;   if (%c1 != 0)
;   {
;      if (%c2 != 0)
;      {
;         goto for.inc290;
;      }
;   }
;   %v = (%a)[i1];
;   %in.42522 = (%v > %dmax.02523) ? i1 : %in.42522;
;   %dmax.02523 = (%v > %dmax.02523) ? %v : %dmax.02523;
;   for.inc290:
; END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @rqbr_(double %d, i1 %c1, i1 %c2, ptr %a) local_unnamed_addr #0 {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK:  Reduction list
; CHECK-NEXT:   (FloatMax) Start: double [[DMAX_025230:%.*]] Exit: double [[VP5:%.*]]
; CHECK-NEXT:    Linked values: double [[VP6:%.*]], double [[VP5]], double [[VP7:%.*]], double [[VP_MINMAX_RED_INIT:%.*]], double [[VP_MINMAX_RED_FINAL:%.*]],
;
; CHECK:        (UIntMin) Start: i64 [[IN_425220:%.*]] Exit: i64 [[VP8:%.*]]
; CHECK-NEXT:    Linked values: i64 [[VP9:%.*]], i64 [[VP8]], i64 [[VP10:%.*]], i64 [[VP_MONO_IDX_RED_INIT:%.*]], i64 [[VP_MONO_IDX_RED_FINAL:%.*]],
; CHECK-NEXT:  IsLinearIndex: 1 Parent exit: double [[VP5]]
;
; CHECK:          double [[VP_MINMAX_RED_INIT]] = reduction-init double [[DMAX_025230]]
; CHECK-NEXT:     i64 [[VP_MONO_IDX_RED_INIT]] = reduction-init i64 [[IN_425220]]
;
; CHECK:          i64 [[VP9]] = phi  [ i64 [[VP_MONO_IDX_RED_INIT]], [[BB2:BB[0-9]+]] ],  [ i64 [[VP8]], [[BB3:BB[0-9]+]] ]
; CHECK-NEXT:     double [[VP6]] = phi  [ double [[VP_MINMAX_RED_INIT]], [[BB2]] ],  [ double [[VP5]], [[BB3]] ]
; CHECK:            ptr [[VP_SUBSCRIPT:%.*]] = subscript ptr [[A0:%.*]] i64 [[VP12:%.*]]
; CHECK-NEXT:       double [[VP_LOAD:%.*]] = load ptr [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i1 [[VP13:%.*]] = fcmp ogt double [[VP_LOAD]] double [[VP6]]
; CHECK-NEXT:       i64 [[VP10]] = select i1 [[VP13]] i64 [[VP12]] i64 [[VP9]]
; CHECK-NEXT:       i1 [[VP14:%.*]] = fcmp ogt double [[VP_LOAD]] double [[VP6]]
; CHECK-NEXT:       double [[VP7]] = select i1 [[VP14]] double [[VP_LOAD]] double [[VP6]]
;
; CHECK:          i64 [[VP8]] = phi  [ i64 [[VP9]], [[BB6:BB[0-9]+]] ],  [ i64 [[VP10]], [[BB5:BB[0-9]+]] ]
; CHECK-NEXT:     double [[VP5]] = phi  [ double [[VP6]], [[BB6]] ],  [ double [[VP7]], [[BB5]] ]
;
; CHECK:          double [[VP_MINMAX_RED_FINAL]] = reduction-final{fmax} double [[VP5]]
; CHECK-NEXT:     i64 [[VP_MONO_IDX_RED_FINAL]] = reduction-final{u_umin} i64 [[VP8]] double [[VP5]] double [[VP_MINMAX_RED_FINAL]]
;
; CHECK:       External Uses:
; CHECK-NEXT:  Id: 0   i64 [[VP_MONO_IDX_RED_FINAL]] -> [[VP16:%.*]] = {%in.42522}
;
entry:
  br label %for.body271.preheader

for.body271.preheader:                            ; preds = %L23055
  br label %for.body271

for.body271:                                      ; preds = %for.inc290, %for.body271.preheader
  %indvars.iv2802 = phi i64 [ 0, %for.body271.preheader ], [ %indvars.iv2802.inc, %for.inc290 ]
  %dmax.02523 = phi double [ %d, %for.body271.preheader ], [ %dmax.1, %for.inc290 ]
  %in.42522 = phi i64 [ 0, %for.body271.preheader ], [ %in.5, %for.inc290 ]
  br i1 %c1, label %if.then278, label %if.end285

if.then278:                                       ; preds = %for.body271
  br i1 %c2,  label %if.end282, label %for.inc290

if.end282:                                        ; preds = %if.then278
  br label %if.end285

if.end285:                                        ; preds = %if.end282, %for.body271
  %gep = getelementptr double, ptr %a, i64 %indvars.iv2802
  %v = load double, ptr %gep
  %cmp286 = fcmp fast ogt double %v, %dmax.02523
  %0 = select i1 %cmp286, i64 %indvars.iv2802, i64 %in.42522
  %1 = select i1 %cmp286, double %v, double %dmax.02523
  br label %for.inc290

for.inc290:                                       ; preds = %if.end285, %if.then278
  %in.5 = phi i64 [ %in.42522, %if.then278 ], [ %0, %if.end285 ]
  %dmax.1 = phi double [ %dmax.02523, %if.then278 ], [ %1, %if.end285 ]
  %indvars.iv2802.inc = add i64 %indvars.iv2802, 1
  %exitcond2807 = icmp eq i64 %indvars.iv2802.inc, 1000
  br i1 %exitcond2807, label %for.cond268.for.end292_crit_edge, label %for.body271

for.cond268.for.end292_crit_edge:                 ; preds = %for.inc290
  %in.5.lcssa = phi i64 [ %in.5, %for.inc290 ]
  br label %for.end292

for.end292:                                       ; preds = %L23055
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }

