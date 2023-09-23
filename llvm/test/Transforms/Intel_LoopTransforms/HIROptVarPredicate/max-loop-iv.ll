
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the analysis process doesn't produce an assertion
; when analyzing an If condition (instruction <49>) that is inside the max loop
; level (9).

; HIR before transformation

; <0>          BEGIN REGION { }
; <139>              + DO i1 = 0, 98, 1   <DO_LOOP>
; <140>              |   + DO i2 = 0, 98, 1   <DO_LOOP>
; <141>              |   |   + DO i3 = 0, 98, 1   <DO_LOOP>
; <142>              |   |   |   + DO i4 = 0, 98, 1   <DO_LOOP>
; <143>              |   |   |   |   + DO i5 = 0, 98, 1   <DO_LOOP>
; <144>              |   |   |   |   |   + DO i6 = 0, 98, 1   <DO_LOOP>
; <145>              |   |   |   |   |   |   + DO i7 = 0, 98, 1   <DO_LOOP>
; <146>              |   |   |   |   |   |   |   + DO i8 = 0, 98, 1   <DO_LOOP>
; <147>              |   |   |   |   |   |   |   |   + DO i9 = 0, %n + -1, 1   <DO_LOOP>
; <49>               |   |   |   |   |   |   |   |   |   if (i9 < %d)
; <49>               |   |   |   |   |   |   |   |   |   {
; <54>               |   |   |   |   |   |   |   |   |      (%p)[i9] = i9;
; <49>               |   |   |   |   |   |   |   |   |   }
; <49>               |   |   |   |   |   |   |   |   |   else
; <49>               |   |   |   |   |   |   |   |   |   {
; <58>               |   |   |   |   |   |   |   |   |      (%q)[i9] = i9;
; <49>               |   |   |   |   |   |   |   |   |   }
; <147>              |   |   |   |   |   |   |   |   + END LOOP
; <146>              |   |   |   |   |   |   |   + END LOOP
; <145>              |   |   |   |   |   |   + END LOOP
; <144>              |   |   |   |   |   + END LOOP
; <143>              |   |   |   |   + END LOOP
; <142>              |   |   |   + END LOOP
; <141>              |   |   + END LOOP
; <140>              |   + END LOOP
; <139>              + END LOOP
; <0>          END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   + DO i4 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   |   + DO i5 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   + DO i6 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   |   + DO i7 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   |   |   + DO i8 = 0, 98, 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   |   |   |   if (%n > 0)
; CHECK:       |   |   |   |   |   |   |   |   {
; CHECK:       |   |   |   |   |   |   |   |      + DO i9 = 0, smin((-1 + %n), (-1 + %d)), 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   |   |   |      |   (%p)[i9] = i9;
; CHECK:       |   |   |   |   |   |   |   |      + END LOOP
; CHECK:       |   |   |   |   |   |   |   |
; CHECK:       |   |   |   |   |   |   |   |
; CHECK:       |   |   |   |   |   |   |   |      + DO i9 = 0, %n + -1 * smax(0, %d) + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   |   |   |   |   |      |   (%q)[i9 + smax(0, %d)] = i9 + smax(0, %d);
; CHECK:       |   |   |   |   |   |   |   |      + END LOOP
; CHECK:       |   |   |   |   |   |   |   |   }
; CHECK:       |   |   |   |   |   |   |   + END LOOP
; CHECK:       |   |   |   |   |   |   + END LOOP
; CHECK:       |   |   |   |   |   + END LOOP
; CHECK:       |   |   |   |   + END LOOP
; CHECK:       |   |   |   + END LOOP
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION


;Module Before HIR
; ModuleID = '/tmp/ifx0896834780xyofHP/ifxiUmZcE.bc'
source_filename = "/tmp/ifx0896834780xyofHP/ifxiUmZcE.bc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"


define void @foo(ptr nocapture %p, ptr nocapture %q, i64 %n, i64 %d) {
do.body01.preheader:
  br label %do.body01

do.body01:
  %indvars.iv01 = phi i64 [ 1, %do.body01.preheader ], [ %indvars.iv01.next, %do.end_01 ]
  br label %do.body02.preheader

do.body02.preheader:
  br label %do.body02

do.body02:
  %indvars.iv02 = phi i64 [ 1, %do.body02.preheader ], [ %indvars.iv02.next, %do.end_02 ]
  br label %do.body03.preheader

do.body03.preheader:
  br label %do.body03

do.body03:
  %indvars.iv03 = phi i64 [ 1, %do.body03.preheader ], [ %indvars.iv03.next, %do.end_03 ]
  br label %do.body04.preheader

do.body04.preheader:
  br label %do.body04

do.body04:
  %indvars.iv04 = phi i64 [ 1, %do.body04.preheader ], [ %indvars.iv04.next, %do.end_04 ]
  br label %do.body05.preheader

do.body05.preheader:
  br label %do.body05

do.body05:
  %indvars.iv05 = phi i64 [ 1, %do.body05.preheader ], [ %indvars.iv05.next, %do.end_05 ]
  br label %do.body06.preheader

do.body06.preheader:
  br label %do.body06

do.body06:
  %indvars.iv06 = phi i64 [ 1, %do.body06.preheader ], [ %indvars.iv06.next, %do.end_06 ]
  br label %do.body07.preheader

do.body07.preheader:
  br label %do.body07

do.body07:
  %indvars.iv07 = phi i64 [ 1, %do.body07.preheader ], [ %indvars.iv07.next, %do.end_07 ]
  br label %do.body08.preheader

do.body08.preheader:
  br label %do.body08

do.body08:
  %indvars.iv08 = phi i64 [ 1, %do.body08.preheader ], [ %indvars.iv08.next, %do.end_08 ]
  br label %entry.loop

entry.loop:
  %cmp9 = icmp sgt i64 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %do.end_08

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %j.010 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %cmp1 = icmp slt i64 %j.010, %d
  %conv = trunc i64 %j.010 to i32
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %j.010
  store i32 %conv, ptr %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %q, i64 %j.010
  store i32 %conv, ptr %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i64 %j.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %do.end_08

do.end_08:
  %indvars.iv08.next = add nuw nsw i64 %indvars.iv08, 1
  %exitcond08 = icmp eq i64 %indvars.iv08.next, 100
  br i1 %exitcond08, label %do.end_08.loopexit, label %do.body08

do.end_08.loopexit:
  br label %do.end_07

do.end_07:
  %indvars.iv07.next = add nuw nsw i64 %indvars.iv07, 1
  %exitcond07 = icmp eq i64 %indvars.iv07.next, 100
  br i1 %exitcond07, label %do.end_07.loopexit, label %do.body07

do.end_07.loopexit:
  br label %do.end_06

do.end_06:
  %indvars.iv06.next = add nuw nsw i64 %indvars.iv06, 1
  %exitcond06 = icmp eq i64 %indvars.iv06.next, 100
  br i1 %exitcond06, label %do.end_06.loopexit, label %do.body06

do.end_06.loopexit:
  br label %do.end_05

do.end_05:
  %indvars.iv05.next = add nuw nsw i64 %indvars.iv05, 1
  %exitcond05 = icmp eq i64 %indvars.iv05.next, 100
  br i1 %exitcond05, label %do.end_05.loopexit, label %do.body05

do.end_05.loopexit:
  br label %do.end_04

do.end_04:
  %indvars.iv04.next = add nuw nsw i64 %indvars.iv04, 1
  %exitcond04 = icmp eq i64 %indvars.iv04.next, 100
  br i1 %exitcond04, label %do.end_04.loopexit, label %do.body04

do.end_04.loopexit:
  br label %do.end_03

do.end_03:
  %indvars.iv03.next = add nuw nsw i64 %indvars.iv03, 1
  %exitcond03 = icmp eq i64 %indvars.iv03.next, 100
  br i1 %exitcond03, label %do.end_03.loopexit, label %do.body03

do.end_03.loopexit:
  br label %do.end_02

do.end_02:
  %indvars.iv02.next = add nuw nsw i64 %indvars.iv02, 1
  %exitcond02 = icmp eq i64 %indvars.iv02.next, 100
  br i1 %exitcond02, label %do.end_02.loopexit, label %do.body02

do.end_02.loopexit:
  br label %do.end_01

do.end_01:
  %indvars.iv01.next = add nuw nsw i64 %indvars.iv01, 1
  %exitcond01 = icmp eq i64 %indvars.iv01.next, 100
  br i1 %exitcond01, label %do.end_01.loopexit, label %do.body01

do.end_01.loopexit:
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
