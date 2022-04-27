; RUN: opt -hir-create-function-level-region -hir-details -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -hir-details -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that cleanup phase is able to remove identical nested ifs so that
; we are able to set ZTTs for both sibling loops with deferred ZTT processing.

; The ZTT is deferred to be processed after parsing because it contains instruction
; %add in the else case. This instruction is unused and eliminated by parser.

; Dump before  showing nested ZTTs-

; if (0x0 true 0x0)    << Deferred outer ZTT
; {
;    for.body:         << LOOP1
;    if (0x0 true 0x0)
;    {
;    }
;    else
;    {
;       goto for.body;
;    }
;    if (0x0 true 0x0) << Inner identical ZTT
;    {
;       for.body3:     << LOOP2
;       if (0x0 true 0x0)
;       {
;       }
;       else
;       {
;          goto for.body3;
;       }
;    }
; } else {
;    0x0 = 0x0  +  0x0;
; }


; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = i1;
; CHECK: + END LOOP

; CHECK: + Ztt: if (%n > 0)
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = %m;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  %wide.trip.count2325 = zext i32 %n to i64
  br i1 %cmp19, label %for.body.preheader, label %unused

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv21 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next22, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv21
  %0 = trunc i64 %indvars.iv21 to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next22, %wide.trip.count2325
  br i1 %exitcond24, label %for.cond1.preheader.loopexit, label %for.body

for.cond1.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.loopexit
  br i1 %cmp19, label %for.body3.preheader, label %for.end8

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %m, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count2325
  br i1 %exitcond, label %for.end8.loopexit, label %for.body3

for.end8.loopexit:                                ; preds = %for.body3
  br label %for.end8

unused:
  %add = add nsw i32 %n, %m
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %for.cond1.preheader, %entry
  ret void
}

