; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-memory-reduction-sinking,print<hir>" -hir-details 2>&1 < %s | FileCheck %s

; Verify that the reduction temps (%tmp and %tmp3) are marked as non-linear in postexit.
; They were being marked as def@1 and verification was failing.

; Print Before

; CHECK: + DO i64 i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |   + DO i32 i2 = 0, 5, 1   <DO_LOOP>
; CHECK: |   |   %2 = (@a)[0];
; CHECK: |   |   (@a)[0] = i1 + %2;
; CHECK: |   |   %3 = (%0)[i1];
; CHECK: |   |   (%0)[i1] = i1 + %3;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Print After

; CHECK: + DO i64 i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |      %tmp = 0;
; CHECK: |      %tmp3 = 0;
; CHECK: |   + DO i32 i2 = 0, 5, 1   <DO_LOOP>
; CHECK: |   |   %tmp3 = %tmp3  +  i1;
; CHECK: |   |   %tmp = %tmp  +  i1;
; CHECK: |   + END LOOP
; CHECK: |      %2 = (@a)[0];
; CHECK: |      (@a)[0] = %2 + %tmp3;
; CHECK: |         <BLOB> NON-LINEAR i32 %tmp3
; CHECK: |      %3 = (%0)[i1];
; CHECK: |      (%0)[i1] = %3 + %tmp;
; CHECK: |         <BLOB> NON-LINEAR i32 %tmp
; CHECK: + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global ptr null, align 8

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @c() local_unnamed_addr #0 {
entry:
  %0 = load ptr, ptr @b, align 8
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc5
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc5 ]
  %ptridx = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %e.012 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %2 = load i32, ptr @a, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr @a, align 4
  %3 = load i32, ptr %ptridx, align 4
  %add4 = add nsw i32 %3, %1
  store i32 %add4, ptr %ptridx, align 4
  %inc = add nuw nsw i32 %e.012, 1
  %exitcond = icmp eq i32 %inc, 6
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond14 = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond14, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  ret void
}

