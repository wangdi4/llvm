; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s -check-prefix=NOOPT 
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -hir-opt-var-predicate-tc-threshold=1 -aa-pipeline="basic-aa" -S -disable-output  < %s 2>&1 | FileCheck %s -check-prefix=OPT

; Check that the following loop transformed as expected and no asserts appeared during HIR invalidation.

; BEGIN REGION { }
;       + DO i1 = 0, 3, 1   <DO_LOOP>
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   if (i1 != 0)
;       |   |   {
;       |   |      + DO i3 = 0, 3, 1   <DO_LOOP>
;       |   |      |   if (i3 != 0)
;       |   |      |   {
;       |   |      |      @_Z4copyj();
;       |   |      |   }
;       |   |      |   @_Z4copyj();
;       |   |      + END LOOP
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; If HIROptVarPredicate switch TCThreshold is set to 1, optimization happens.
; OPT: BEGIN REGION { modified }
; OPT:       + DO i1 = 0, 2, 1   <DO_LOOP>
; OPT:       |   + DO i2 = 0, 9, 1   <DO_LOOP>
; OPT:       |   |   @_Z4copyj();
; OPT:       |   |
; OPT:       |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; OPT:       |   |   |   @_Z4copyj();
; OPT:       |   |   |   @_Z4copyj();
; OPT:       |   |   + END LOOP
; OPT:       |   + END LOOP
; OPT:       + END LOOP
; OPT: END REGION

; Default value for small loop TC is 5. No optimization happens.
; NOOPT:  BEGIN REGION { }
; NOOPT:  + DO i1 = 0, 3, 1   <DO_LOOP>
; NOOPT:  |   + DO i2 = 0, 9, 1   <DO_LOOP>
; NOOPT:  |   |   if (i1 != 0)
; NOOPT:  |   |   {
; NOOPT:  |   |      + DO i3 = 0, 3, 1   <DO_LOOP>
; NOOPT:  |   |      |   if (i3 != 0)



source_filename = "atg_CMPLRLLVM-24517_cpp.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end27, %entry
  %a.066 = phi i32 [ 0, %entry ], [ %inc29, %for.end27 ]
  %tobool1254 = icmp eq i32 %a.066, 0
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.end, %for.cond1.preheader
  %c.062 = phi i32 [ 0, %for.cond1.preheader ], [ %inc26, %for.end ]
  br label %for.body6.lr.ph.split.us

for.body6.lr.ph.split.us:                         ; preds = %for.cond4.preheader
  br i1 %tobool1254, label %for.end, label %for.body6.us.preheader

for.body6.us.preheader:                           ; preds = %for.body6.lr.ph.split.us
  br label %for.body6.us

for.body6.us:                                     ; preds = %if.end.us, %for.body6.us.preheader
  %e.059.us = phi i32 [ %inc.us, %if.end.us ], [ 0, %for.body6.us.preheader ]
  %tobool18.us = icmp eq i32 %e.059.us, 0
  br i1 %tobool18.us, label %if.end.us, label %if.then19.us

if.then19.us:                                     ; preds = %for.body6.us
  tail call void @_Z4copyj()
  br label %if.end.us

if.end.us:                                        ; preds = %if.then19.us, %for.body6.us
  tail call void @_Z4copyj()
  %inc.us = add nuw nsw i32 %e.059.us, 1
  %exitcond94 = icmp eq i32 %inc.us, 4
  br i1 %exitcond94, label %for.end.loopexit, label %for.body6.us

for.end.loopexit:                                 ; preds = %if.end.us
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body6.lr.ph.split.us
  %inc26 = add nuw nsw i32 %c.062, 1
  %exitcond95 = icmp eq i32 %inc26, 10
  br i1 %exitcond95, label %for.end27, label %for.cond4.preheader

for.end27:                                        ; preds = %for.end
  %inc29 = add nuw nsw i32 %a.066, 1
  %exitcond96 = icmp eq i32 %inc29, 4
  br i1 %exitcond96, label %for.end30, label %for.cond1.preheader

for.end30:                                        ; preds = %for.end27
  ret void
}

declare dso_local void @_Z4copyj() local_unnamed_addr #0

