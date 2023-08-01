; RUN: opt -S -disable-output -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that loop will not be MV'ed because is contans a switch, which is against a cost model.

; BEGIN REGION { }
;       + DO i1 = 0, 999, 1   <DO_LOOP>
;       |   + DO i2 = 0, 999, 1   <DO_LOOP>
;       |     ... switch ...
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: Function: foo

; CHECK-NOT: if
; CHECK: DO i1
; CHECK: DO i2
; CHECK: END LOOP
; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %a, ptr nocapture %p, i32 %n) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %arrayidx12 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv.next32
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv31
  %0 = trunc i64 %indvars.iv31 to i32
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.inc
  %exitcond33 = icmp eq i64 %indvars.iv.next32, 1000
  br i1 %exitcond33, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  switch i32 %n, label %for.inc [
    i32 16, label %sw.bb
    i32 15, label %sw.bb5
    i32 0, label %sw.bb8
  ]

sw.bb:                                            ; preds = %for.body4
  store i32 %0, ptr %arrayidx, align 4
  br label %for.inc

sw.bb5:                                           ; preds = %for.body4
  %arrayidx7 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx7, align 4
  br label %for.inc

sw.bb8:                                           ; preds = %for.body4
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx10, align 4
  store i32 %2, ptr %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %sw.bb8, %sw.bb5, %sw.bb, %for.body4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

