; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; This command checks that -hir-ssa-deconstruction invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -hir-cost-model-throttling=0 -disable-output  2>&1 | FileCheck %s
; RUN: opt %s -passes="convert-to-subscript,hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -hir-cost-model-throttling=0 -disable-output  2>&1 | FileCheck %s

; Check parsing output for reduction chain in the loopnest
; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, sext.i32.i64((-1 + %m)), 1   <DO_LOOP>
; CHECK: |   |   %[[T0:.*]] = (@A)[0][i2][i1];
; CHECK: |   |   %[[T1:.*]] = %[[T0]]  +  %[[T1]];
; CHECK: |   |   %[[T2:.*]] = %[[T1]];
; CHECK: |   |   %[[T3:.*]] = (@B)[0][i2][i1];
; CHECK: |   |   %[[T1]] = %[[T1]]  +  %[[T3]];
; CHECK: |   |   %call = @foo1(%[[T2]]);
; CHECK: |   + END LOOP
; CHECK: |      %r.035 = %t.034.out;
; CHECK: |
; CHECK: |   %t.034.out1 = %t.034;
; CHECK: |   %r.035.out = %r.035;
; CHECK: + END LOOP


; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -hir-framework-debug=parser -disable-output -hir-details 2>&1 | FileCheck %s -check-prefix=DETAIL

; Check loop livein/liveouts symbases.

; Collect region liveout symbases.
; DETAIL: LiveOuts:
; DETAIL-DAG: %r.035.out(sym:[[LIVEOUT1:[0-9]+]])
; DETAIL-DAG: %t.034.out1(sym:[[LIVEOUT2:[0-9]+]])

; DETAIL-NOT: DO i
; Collect i1 loop liveins.
; DETAIL: LiveIn symbases: [[I1LIVEIN1:[0-9]+]], [[I1LIVEIN2:[0-9]+]]

; Check that region liveouts are the same as i1 loop's liveouts.
; DETAIL: LiveOut symbases:
; DETAIL-DAG: [[LIVEOUT1]]
; DETAIL-DAG: [[LIVEOUT2]]

; Check that i2 loop shares a livein with i1 loop.
; DETAIL: LiveIn symbases: [[I1LIVEIN2]]

; Collect i2 loop liveouts.
; DETAIL-NEXT: LiveOut symbases: [[I1LIVEIN2]], [[I2LIVEOUT1:[0-9]+]]

; Check that %t.034 is correctly set as livein and non-linear in the i2 loop.
; DETAIL: DO i64 i2
; DETAIL: %t.034.out = %t.034
; DETAIL-NEXT: <LVAL-REG> NON-LINEAR i32 %t.034.out {sb:[[I2LIVEOUT1]]}
; DETAIL-NEXT: <RVAL-REG> NON-LINEAR i32 %t.034 {sb:[[I1LIVEIN2]]}

; Check that %r.035 is correctly set as livein to i1 loop.
; DETAIL: %r.035.out = %r.035
; DETAIL: <RVAL-REG> NON-LINEAR i32 %r.035 {sb:[[I1LIVEIN1]]}


; ModuleID = 'red_nest2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x i32]] zeroinitializer, align 16
@B = common global [1000 x [1000 x i32]] zeroinitializer, align 16

define void @foo(i32 %n, i32 %m) {
entry:
  %cmp32 = icmp sgt i32 %n, 0
  br i1 %cmp32, label %for.cond1.preheader.lr.ph, label %for.end13

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp228 = icmp sgt i32 %m, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %for.cond1.preheader.lr.ph
  %indvars.iv38 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next39, %for.inc11 ]
  %r.035 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %r.1.lcssa, %for.inc11 ]
  %t.034 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %t.1.lcssa, %for.inc11 ]
  br i1 %cmp228, label %for.body3.preheader, label %for.inc11

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %t.130 = phi i32 [ %add10, %for.body3 ], [ %t.034, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x i32]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv38
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, %t.130
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv38
  %1 = load i32, ptr %arrayidx9, align 4
  %add10 = add nsw i32 %add, %1
  %call = tail call i32 @foo1(i32 %add)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.inc11.loopexit, label %for.body3

for.inc11.loopexit:                               ; preds = %for.body3
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond1.preheader
  %r.1.lcssa = phi i32 [ %r.035, %for.cond1.preheader ], [ %add, %for.inc11.loopexit ]
  %t.1.lcssa = phi i32 [ %t.034, %for.cond1.preheader ], [ %add10, %for.inc11.loopexit ]
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %lftr.wideiv40 = trunc i64 %indvars.iv.next39 to i32
  %exitcond41 = icmp eq i32 %lftr.wideiv40, %n
  br i1 %exitcond41, label %for.end13.loopexit, label %for.cond1.preheader

for.end13.loopexit:                               ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %entry
  %r.0.lcssa = phi i32 [ 1, %entry ], [ %r.1.lcssa, %for.end13.loopexit ]
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.end13.loopexit ]
  %add14 = add nsw i32 %r.0.lcssa, %t.0.lcssa
  store i32 %add14, ptr getelementptr inbounds ([1000 x [1000 x i32]], ptr @A, i64 0, i64 5, i64 5), align 4
  ret void
}

declare i32 @foo1(i32)
