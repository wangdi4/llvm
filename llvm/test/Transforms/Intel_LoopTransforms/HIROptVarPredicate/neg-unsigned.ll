; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -S -disable-output  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -print-changed -disable-output  < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Test checks that OptVarPredicate pass does not hoist out 'if'
; with unsigned predicate if LHS could have negative value.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, sext.i32.i64(%M) + -2, 1   <DO_LOOP>
;                  |   if (i1 + -4 <u 3)
;                  |   {
;                  |      (%A)[i1 + 1] = i1 + 1;
;                  |   }
;                  + END LOOP
;            END REGION

; Check that no optimization happen.
; CHECK: BEGIN REGION { }

; Verify that pass is not dumped with print-changed if it bails out.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIROptVarPredicate


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %A, i32 noundef %M, i32 noundef %N) local_unnamed_addr {
entry:
  %conv = sext i32 %M to i64
  %cmp12 = icmp ugt i32 %M, 1
  br i1 %cmp12, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.013 = phi i64 [ %inc, %for.inc ], [ 1, %for.body.preheader ]
  %sub = add i64 %i.013, -5
  %cmp2 = icmp ult i64 %sub, 3
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %conv4 = trunc i64 %i.013 to i32
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %i.013
  store i32 %conv4, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw i64 %i.013, 1
  %exitcond.not = icmp eq i64 %inc, %conv
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %conv
  %0 = load i32, ptr %arrayidx5, align 4
  ret i32 %0
}

