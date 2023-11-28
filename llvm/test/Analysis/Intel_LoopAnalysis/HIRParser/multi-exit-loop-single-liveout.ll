; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check that we correctly parse the multi-exit loop and set the liveout value %0.

; CHECK: LiveOuts: %0

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %0 = (%A)[i1];
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      goto for.end.loopexit;
; CHECK: |   }
; CHECK: + END LOOP

; Check CG for liveout value %0
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG


; CHECK-CG: %t.1.ph = phi i32 [ %0, %for.cond ], [ %0, %for.body.split ], [ [[LIVEOUTLOAD1:.*]], %[[NORMALEXIT:.*]] ], [ [[LIVEOUTLOAD2:.*]], %[[EARLYEXIT:.*]] ]

; CHECK-CG: region.0

; Loop header BB should jump to early exit if compare evalutates to true.
; CHECK-CG: {{loop.*:}}
; CHECK-CG: br i1 {{.*}}, label %[[EARLYEXIT:.*]], label {{.*}}

; CHECK-CG: [[EARLYEXIT]]:
; CHECK-CG: [[LIVEOUTLOAD2]] = load i32, ptr [[LIVEOUTVAL:.*]]

; CHECK-CG: [[NORMALEXIT]]:
; CHECK-CG: [[LIVEOUTLOAD1]] = load i32, ptr [[LIVEOUTVAL]]


; ModuleID = 'multi-exit3_1.ll'
source_filename = "multi-exit3.ll"

define i32 @foo(ptr nocapture readonly %A, i32 %n) local_unnamed_addr {
entry:
  %cmp10 = icmp slt i32 0, %n
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.cond, %for.body.preheader
  %i.011 = phi i32 [ %inc, %for.cond ], [ 0, %for.body.preheader ]
  %idxprom = sext i32 %i.011 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %cmp3 = icmp sgt i32 %0, 0
  %inc = add nsw i32 %i.011, 1
  br i1 %cmp3, label %for.end.loopexit, label %for.cond

for.cond:                                         ; preds = %for.body
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body, %for.cond
  %t.1.ph = phi i32 [ %0, %for.cond ], [ %0, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.1 = phi i32 [ 0, %entry ], [ %t.1.ph, %for.end.loopexit ]
  ret i32 %t.1
}

