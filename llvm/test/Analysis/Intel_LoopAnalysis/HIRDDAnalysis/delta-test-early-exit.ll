; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s

; Test checks that on-demand DD doesn't fail when DD delta test code falls on early exit.

; CHECK: DD graph for function foo
; CHECK: %v2 --> %v2 FLOW (= =)

@w = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

define dso_local i32 @foo(i32* %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 29, %entry ], [ %indvars.iv.next, %for.inc ]
  %jh = phi i32 [ 29, %entry ], [ %dec, %for.inc ]
  br label %for.cond2.preheader.preheader

for.cond2.preheader.preheader:
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.preheader, %for.cond.cleanup2
  %indvars.iv1 = phi i64 [ %indvars.iv.next1, %for.cond.cleanup2 ], [ %indvars.iv, %for.cond2.preheader.preheader ]
  %v1 = trunc i64 %indvars.iv1 to i32
  %add = add i64 %indvars.iv1, 1
  %idxprom = and i64 %add, 4294967295
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @w, i64 0, i64 %idxprom, i64 %indvars.iv1
  %sub = add nsw i32 %v1, -1
  %idxprom1 = zext i32 %sub to i64
  br label %for.body

for.cond.cleanup2:                               ; preds = %for.body
  %cmp2 = icmp ugt i32 %sub, 1
  %indvars.iv.next1 = add nsw i64 %indvars.iv1, -1
  br i1 %cmp2, label %for.cond2.preheader, label %for.end

for.body:                                       ; preds = %for.cond2.preheader, %for.body
  %indvars.iv2 = phi i64 [ 1, %for.cond2.preheader ], [ %indvars.iv.next2, %for.body ]
  %v2 = load i32, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @w, i64 0, i64 %indvars.iv2, i64 %idxprom1
  store i32 %v2, i32* %arrayidx1, align 4
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond = icmp eq i64 %indvars.iv.next2, 10
  br i1 %exitcond, label %for.cond.cleanup2, label %for.body

for.end:                                        ; preds = %for.cond.cleanup2
  %tobool = icmp ugt i32 %v2, 2
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.end
  store i32 5, i32* %n, align 4
  br label %for.inc

for.inc:                                        ; preds = %for.end, %if.then
  %dec = add nsw i32 %jh, -1
  %cmp = icmp ugt i32 %jh, 6
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.inc
  ret i32 0
}


