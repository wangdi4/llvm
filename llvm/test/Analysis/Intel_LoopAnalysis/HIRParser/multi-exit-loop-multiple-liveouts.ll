; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check that we correctly parse the multi-exit loop and set the liveout values %t.0.be && %t.021.

; CHECK: LiveOuts:
; CHECK-DAG: %t.021.out
; CHECK-DAG: %t.0.be

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %t.021.out = %t.021;
; CHECK: |   %1 = (%B)[i1];
; CHECK: |   if (%1 <= 0)
; CHECK: |   {
; CHECK: |      goto for.end.loopexit;
; CHECK: |   }
; CHECK: |   %2 = (%A)[i1];
; CHECK: |   %t.0.be = %2;
; CHECK: |   if (%2 <= 0)
; CHECK: |   {
; CHECK: |      %3 = (%A)[i1 + 1];
; CHECK: |      %t.0.be = %3;
; CHECK: |   }
; CHECK: |   %t.021 = %t.0.be;
; CHECK: + END LOOP

; Check CG for liveout values.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG

; CHECK-CG: %t.0.lcssa.ph = phi i32 [ %t.0.be, %for.cond.backedge ], [ %t.021.out, %for.body.split ], [ [[LIVEOUT1LOAD:.*]], %[[NORMALEXIT:.*]] ], [ [[LIVEOUT2LOAD:.*]], %[[EARLYEXIT:.*]] ]

; CHECK-CG: region.0

; Verify that %t.021.out, which is set in the first statement of the loop, is live out of the early exit.
; CHECK-CG: {{loop.*:}}
; CHECK-CG: store i32 {{.*}}, ptr [[EARLY_LIVEOUT_VAL:.*]]

; Loop header BB should jump to early exit if compare evalutates to true.
; CHECK-CG: br i1 {{.*}}, label %[[EARLYEXIT]], label {{.*}}

; CHECK-CG: [[EARLYEXIT]]:
; CHECK-CG: [[LIVEOUT2LOAD]] = load i32, ptr [[EARLY_LIVEOUT_VAL]]

; Verify that %t.0.be, which is used in the last statement of the loop, is live out of the normal exit.
; CHECK-CG: {{ifmerge.*:}}
; CHECK-CG: {{ifmerge.*:}}
; CHECK-CG: {{%t.*}} = load i32, ptr [[NORMAL_LIVEOUT_VAL:.*]]

; CHECK-CG: [[NORMALEXIT]]:
; CHECK-CG: [[LIVEOUT1LOAD]] = load i32, ptr [[NORMAL_LIVEOUT_VAL]]



define i32 @foo(ptr nocapture readonly %A, ptr nocapture readonly %B, i32 %n) {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.cond.backedge
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.cond.backedge ]
  %t.021 = phi i32 [ 0, %for.body.preheader ], [ %t.0.be, %for.cond.backedge ]
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %1, 0
  br i1 %cmp1, label %if.then, label %for.end.loopexit

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %2, 0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp4, label %for.cond.backedge, label %if.else

if.else:                                          ; preds = %if.then
  %arrayidx9 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %3 = load i32, ptr %arrayidx9, align 4
  br label %for.cond.backedge

for.cond.backedge:                                ; preds = %if.then, %if.else
  %t.0.be = phi i32 [ %2, %if.then ], [ %3, %if.else ]
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body, %for.cond.backedge
  %t.0.lcssa.ph = phi i32 [ %t.0.be, %for.cond.backedge ], [ %t.021, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %t.0.lcssa.ph, %for.end.loopexit ]
  ret i32 %t.0.lcssa
}

