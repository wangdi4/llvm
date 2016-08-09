; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; HIR-
; + DO i1 = 0, 49, 1   <DO_LOOP>
; |   + DO i2 = 0, 38, 1   <DO_LOOP>
; |   |   + DO i3 = 0, zext.i32.i64((trunc.i64.i32(%indvars.iv) + umax(-2, (-1 * trunc.i64.i32(%indvars.iv))))), 1   <DO_LOOP>
; |   |   |   %1 = (%s)[0][i1 + -1 * i3 + 3][i1 + -1 * i3 + 2];
; |   |   |   (%h5)[0][i1 + -1 * i3 + 2] = %1;
; |   |   + END LOOP
; |   |   %mul66 = %1  *  %1;
; |   + END LOOP
; |   %indvars.iv = i1 + 3;
; + END LOOP

; Check livein/liveout of the loopnest verifying that %1 is marked live out of i3 loop and %mul66 is marked live out of i2 loop.

; Collect region liveout symbases.
; CHECK: LiveOuts:
; CHECK: %mul66.lcssa(sym:[[LIVEOUT:[0-9]+]])

; CHECK-NOT: DO i
; Collect i1 loop liveins.
; CHECK: LiveIn symbases: [[I1LIVEIN1:[0-9]+]], [[I1LIVEIN2:[0-9]+]], [[I1LIVEIN3:[0-9]+]]

; Check that i1 loop's liveouts are the same as region liveouts.
; CHECK: LiveOut symbases: [[LIVEOUT]]

; Check that i2 loop liveins/liveouts are the same as i1 loop liveins/liveouts.
; CHECK: LiveIn symbases: [[I1LIVEIN1]], [[I1LIVEIN2]], [[I1LIVEIN3]]
; CHECK: LiveOut symbases: [[LIVEOUT]]

; Check that i3 loop liveins are the same as i1 loop liveins.
; CHECK: LiveIn symbases: [[I1LIVEIN1]], [[I1LIVEIN2]], [[I1LIVEIN3]]

; Collect i3 loop liveout.
; CHECK: LiveOut symbases: [[I3LIVEOUT:[0-9]+]]

; Check that %1 is live out of i3 loop.
; CHECK: <LVAL-REG> NON-LINEAR i32 %1 {sb:[[I3LIVEOUT]]}

; Check that %mul66 is live out of i1/i2 loops and the region.
; CHECK: %mul66 = %1  *  %1
; CHECK-NEXT: <LVAL-REG> NON-LINEAR i32 (%1 * %1) {sb:[[LIVEOUT]]}


define i32 @main() #0 {
entry:
  %h5 = alloca [100 x i32], align 16
  %s = alloca [100 x [100 x i32]], align 16
  br label %for.cond43.preheader
 
for.cond43.preheader:                             ; preds = %for.inc100, %entry
  %indvars.iv = phi i64 [ 2, %entry ], [ %indvars.iv.next, %for.inc100 ]
  br label %for.body48.preheader

for.body48.preheader:                             ; preds = %for.cond43.preheader, %for.inc98
  %inc231 = phi i32 [ 1, %for.cond43.preheader ], [ %inc, %for.inc98 ]
  br label %for.body48

for.body48:                                       ; preds = %for.body48.preheader, %for.body48
  %indvars.iv246 = phi i64 [ %indvars.iv, %for.body48.preheader ], [ %indvars.iv.next247, %for.body48 ]
  %0 = trunc i64 %indvars.iv246 to i32
  %add50 = add i64 %indvars.iv246, 1
  %idxprom51 = and i64 %add50, 4294967295
  %arrayidx53 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %s, i64 0, i64 %idxprom51, i64 %indvars.iv246
  %1 = load i32, i32* %arrayidx53, align 4
  %arrayidx55 = getelementptr inbounds [100 x i32], [100 x i32]* %h5, i64 0, i64 %indvars.iv246
  store i32 %1, i32* %arrayidx55, align 4
  %dec96 = add nsw i32 %0, -1
  %cmp47 = icmp ugt i32 %dec96, 1
  %indvars.iv.next247 = add nsw i64 %indvars.iv246, -1
  br i1 %cmp47, label %for.body48, label %for.inc98

for.inc98:                                        ; preds = %for.body48
  %.lcssa = phi i32 [ %1, %for.body48 ]
  %mul66 = mul i32 %.lcssa, %.lcssa
  %inc = add nuw nsw i32 %inc231, 1
  %exitcond248 = icmp eq i32 %inc, 40
  br i1 %exitcond248, label %for.inc100, label %for.body48.preheader

for.inc100:                                       ; preds = %for.inc98
  %mul66.lcssa = phi i32 [ %mul66, %for.inc98 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond249 = icmp eq i64 %indvars.iv.next, 52
  br i1 %exitcond249, label %for.end102, label %for.cond43.preheader

for.end102:
  ret i32 %mul66.lcssa
}
