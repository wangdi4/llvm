; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-loop-distribute-always-stripmine -hir-loop-distribute-skip-vectorization-profitability-check=true -hir-loop-distribute-scex-cost=3 -debug-only=hir-loop-distribute < %s 2>&1 | FileCheck %s

; Verify that we skip distributing the loop using stripmining/scalar-expansaion
; if it is likely a small trip count loop.

;   HIR-
;         BEGIN REGION { }
;               + DO i1 = 0, %it + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;               |   if (%z > %cmp)
;               |   {
;               |      %x.addr.020 = %x.addr.020  +  1;
;               |      %y.addr.019 = %y.addr.019  +  1;
;               |      %0 = (@A)[0][i1 + %s];
;               |      %1 = (@B)[0][%y.addr.019];
;               |      %2 = (@A1)[0][%x.addr.020];
;               |      (@B1)[0][%x.addr.020] = %2 + (%0 * %1);
;               |   }
;               + END LOOP
;         END REGION


; CHECK: LOOP DISTRIBUTION: Skipping distribution of likely small trip count loop using stripmining and scalar-expansion at it doesn't seem profitable
; CHECK-NOT: modified


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B2 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(i32 %x, i32 %y, i32 %z, i32 %cmp, i32 %it, i4 %s) {
entry:
  %cmp2 = icmp sgt i32 %z, %cmp
  %cmp118 = icmp sgt i32 %it, 0
  br i1 %cmp118, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %x.addr.1.lcssa = phi i32 [ %x.addr.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %x.addr.0.lcssa = phi i32 [ %x, %entry ], [ %x.addr.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %x.addr.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.021 = phi i32 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %x.addr.020 = phi i32 [ %x.addr.1, %for.inc ], [ %x, %for.body.preheader ]
  %y.addr.019 = phi i32 [ %y.addr.1, %for.inc ], [ %y, %for.body.preheader ]
  br i1 %cmp2, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %add = add nsw i32 %x.addr.020, 1
  %add3 = add nsw i32 %y.addr.019, 1
  %idxprom = sext i32 %add to i64
  %trunc = trunc i32 %i.021 to i4
  %add4 = add nsw i4 %trunc, %s
  %ext = sext i4 %add4 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %ext
  %0 = load i32, ptr %arrayidx, align 4
  %idxprom4 = sext i32 %add3 to i64
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %idxprom4
  %1 = load i32, ptr %arrayidx5, align 4
  %mul = mul nsw i32 %1, %0
  %arrayidx7 = getelementptr inbounds [10 x i32], ptr @A1, i64 0, i64 %idxprom
  %2 = load i32, ptr %arrayidx7, align 4
  %add8 = add nsw i32 %mul, %2
  %arrayidx10 = getelementptr inbounds [10 x i32], ptr @B1, i64 0, i64 %idxprom
  store i32 %add8, ptr %arrayidx10, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %y.addr.1 = phi i32 [ %add3, %if.then ], [ %y.addr.019, %for.body ]
  %x.addr.1 = phi i32 [ %add, %if.then ], [ %x.addr.020, %for.body ]
  %inc = add nuw nsw i32 %i.021, 1
  %exitcond.not = icmp eq i32 %inc, %it
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

