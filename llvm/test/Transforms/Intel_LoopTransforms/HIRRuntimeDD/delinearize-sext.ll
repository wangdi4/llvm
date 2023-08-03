; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -enable-hir-runtime-dd-delin-sext -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   %add8 = (%p)[%n * i1 + i2 + 1]  +  (%q)[i2];
;       |   |   (%p)[%n * i1 + i2 + %n] = %add8;
;       |   + END LOOP
;       + END LOOP
; END REGION

; Check that ext.i32.i64 indices can be delinearized.

; CHECK: &((%p)[100][99]) >=u &((%q)[0]);
; CHECK: &((%q)[99]) >=u &((%p)[0][1]);
; CHECK: %n > 1 & 100 < %n

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p, ptr nocapture readonly %q, i32 %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv34 = phi i32 [ 0, %entry ], [ %indvars.iv.next35, %for.cond.cleanup3 ]
  %0 = mul nsw i32 %indvars.iv34, %n
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next35 = add nuw nsw i32 %indvars.iv34, 1
  %exitcond37 = icmp eq i32 %indvars.iv.next35, 100
  br i1 %exitcond37, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i32 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %1 = add nsw i32 %indvars.iv, %0
  %2 = add nsw i32 %1, 1
  %s3 = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds float, ptr %p, i64 %s3
  %3 = load float, ptr %arrayidx, align 4
  %indvars.iv.s = zext i32 %indvars.iv to i64
  %arrayidx7 = getelementptr inbounds float, ptr %q, i64 %indvars.iv.s
  %4 = load float, ptr %arrayidx7, align 4
  %add8 = fadd float %3, %4
  %5 = add nsw i32 %1, %n
  %s6 = sext i32 %5 to i64
  %arrayidx13 = getelementptr inbounds float, ptr %p, i64 %s6
  store float %add8, ptr %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond = icmp eq i32 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}
