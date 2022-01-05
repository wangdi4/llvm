; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; RUN: opt < %s -convert-to-subscript -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; This command checks that -hir-ssa-deconstruction invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt < %s -convert-to-subscript -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll 2>&1 | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that the compare instruction is parsed correctly.
; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %small.030.out = %small.030;
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   %1 = (%B)[i1];
; CHECK: |   %cmp3 = %0 < %1;
; CHECK: |   %tobool = %small.030.out != 0;
; CHECK: |   %small.030 = %cmp3  |  %tobool;
; CHECK: |   if (%small.030 == 0)
; CHECK: |   {
; CHECK: |      (%B)[i1] = %0;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      (%A)[i1] = %1;
; CHECK: |   }
; CHECK: + END LOOP


; ModuleID = 'cmp1.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPiS_i(i32* nocapture %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.28 = icmp sgt i32 %n, 0
  br i1 %cmp.28, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %small.030 = phi i8 [ %frombool, %for.inc ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp slt i32 %0, %1
  %tobool = icmp ne i8 %small.030, 0
  %cmp326 = zext i1 %cmp3 to i8
  %tobool27 = zext i1 %tobool to i8
  %frombool = or i8 %cmp326, %tobool27
  %tobool4 = icmp eq i8 %frombool, 0
  br i1 %tobool4, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  store i32 %1, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  store i32 %0, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
