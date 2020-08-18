; Check that DO i2 loop will be transformed into memcpy.

; RUN: opt -scoped-noalias-aa -hir-ssa-deconstruction -disable-output -hir-temp-cleanup -hir-runtime-dd -hir-idiom -print-after=hir-idiom < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa,scoped-noalias-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-idiom,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 7, 1   <DO_LOOP>
;       |   + DO i2 = 0, 7, 1   <DO_LOOP>
;       |   |   (%A)[i1 + i2] = (%B)[i1 + i2];
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK: if (%mv
; CHECK: DO i1
; CHECK:   @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%A)[i1]),  &((i8*)(%B)[i1]),  32,  0);
; CHECK-NOT: DO i2
; CHECK: else
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %A, i32* nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvar
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %0
  %1 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds i32, i32* %A, i64 %0
  store i32 %1, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %indvar.next = add nuw nsw i64 %indvar, 1
  %exitcond22 = icmp eq i64 %indvar.next, 8
  br i1 %exitcond22, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

