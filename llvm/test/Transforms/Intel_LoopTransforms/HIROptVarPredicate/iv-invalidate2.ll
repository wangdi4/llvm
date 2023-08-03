; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s -check-prefix=NOOPT
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -hir-opt-var-predicate-tc-threshold=1 -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s  -check-prefix=OPT

;           BEGIN REGION { }
;                  + DO i1 = 0, 3, 1   <DO_LOOP>
;                  |   + DO i2 = 0, -1 * undef + 3, 1   <DO_LOOP>
;                  |   |   if (undef == 0)
;                  |   |   {
;                  |   |      + DO i3 = 0, 63, 1   <DO_LOOP>
;                  |   |      |   if (i3 != 0)
;                  |   |      |   {
;                  |   |      |      if (i1 != 0)
;                  |   |      |      {
;                  |   |      |         @_Z6printbj();
;                  |   |      |      }
;                  |   |      |   }
;                  |   |      + END LOOP
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; Optimization happens if TC threshold is lowered to 1.
; OPT:     BEGIN REGION { modified }
; OPT:           + DO i1 = 0, 2, 1   <DO_LOOP>
; OPT:           |   + DO i2 = 0, -1 * undef + 3, 1   <DO_LOOP>
; OPT:           |   |   if (undef == 0)
; OPT:           |   |   {
; OPT:           |   |      + DO i3 = 0, 62, 1   <DO_LOOP>
; OPT:           |   |      |   @_Z6printbj();
; OPT:           |   |      + END LOOP
; OPT:           |   |   }
; OPT:           |   + END LOOP
; OPT:           + END LOOP
; OPT:     END REGION

; Default value for TC threshold is 5. No optimization happens for i1 loop.
; NOOPT:     BEGIN REGION { modified }
; NOOPT:           + DO i1 = 0, 3, 1   <DO_LOOP>
; NOOPT:           |   + DO i2 = 0, -1 * undef + 3, 1   <DO_LOOP>
; NOOPT:           |   |   if (undef == 0)
; NOOPT:           |   |   {
; NOOPT:           |   |      + DO i3 = 0, 62, 1   <DO_LOOP>
; NOOPT:           |   |      |   if (i1 != 0)
; NOOPT:           |   |      |   {
; NOOPT:           |   |      |      @_Z6printbj();
; NOOPT:           |   |      |   }
; NOOPT:           |   |      + END LOOP
; NOOPT:           |   |   }
; NOOPT:           |   + END LOOP
; NOOPT:           + END LOOP
; NOOPT:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @main() local_unnamed_addr #0 {
entry:
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc31, %entry
  %indvars.iv66 = phi i64 [ 0, %entry ], [ %indvars.iv.next67, %for.inc31 ]
  %tobool12.not = icmp eq i64 %indvars.iv66, 0
  br label %for.body3

for.body3:                                        ; preds = %for.end, %for.body3.lr.ph
  %d.060 = phi i32 [ undef, %for.body3.lr.ph ], [ %inc29, %for.end ]
  br i1 undef, label %for.end, label %for.body6.preheader

for.body6.preheader:                              ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.inc, %for.body6.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body6.preheader ]
  br label %sw.bb

sw.bb:                                            ; preds = %for.body6
  %tobool10.not = icmp eq i64 %indvars.iv, 0
  br i1 %tobool10.not, label %if.end24, label %if.then11

if.then11:                                        ; preds = %sw.bb
  br i1 %tobool12.not, label %if.else, label %if.then13

if.then13:                                        ; preds = %if.then11
  call void @_Z6printbj()
  br label %if.end24

if.else:                                          ; preds = %if.then11
  br label %if.end24

if.end24:                                         ; preds = %if.else, %if.then13, %sw.bb
  br label %for.inc

for.inc:                                          ; preds = %if.end24
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body6

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body3
  %inc29 = add nuw nsw i32 %d.060, 1
  %exitcond65.not = icmp eq i32 %inc29, 4
  br i1 %exitcond65.not, label %for.inc31, label %for.body3

for.inc31:                                        ; preds = %for.end
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond68.not = icmp eq i64 %indvars.iv.next67, 4
  br i1 %exitcond68.not, label %for.end33, label %for.body3.lr.ph

for.end33:                                        ; preds = %for.inc31
  ret void
}

declare dso_local void @_Z6printbj() local_unnamed_addr #0

