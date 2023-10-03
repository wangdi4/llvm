; RUN: opt -passes='default<O2>' -loopopt -print-changed -disable-output 2>&1 < %s | FileCheck %s

; This test case checks that HIRMemoryReductionSinking inside the condition
; isn't applied until HIROptVarPredicatePass runs first. The reason is that
; HIRMemoryReductionSinking is split in two phases. First, it runs before
; any of opt-predicate passes and the optimization will be applied for the
; reductions outside the conditions. Then, we run the same pass after all
; opt-predicate passes to handle any reduction inside conditions, or the
; reductions that where hoisted out by the opt-predicate passes.

; HIRMemoryReductionSinking should not kick in during the first run
; CHECK-NOT: IR Dump After HIRMemoryReductionSinkingPass

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%t14)[i1];
;       |   if (i1 != %d)
;       |   {
;       |      %t15 = (%t13)[5];
;       |      (%t13)[5] = %t15 + %m;
;       |   }
;       |   if (%0 >u 112)
;       |   {
;       |      %t16 = (%t13)[7];
;       |      (%t13)[7] = %0 + %t16;
;       |   }
;       + END LOOP
; END REGION


; HIR opt-var-predicate should applied to get rid of the first condition
; CHECK: IR Dump After HIROptVarPredicatePass
; CHECK: BEGIN REGION { modified }
; CHECK-NOT: if (i1 != %d)

; HIRLMM should handle the new reductions that got exposed in the new loops
; CHECK: IR Dump After HIRLMMPass
; CHECK: BEGIN REGION { modified }
; CHECK:  %limm = (%t13)[5];
; CHECK:  %limm9 = (%t13)[5];

; HIR after running the second HIRMemoryReductionSinking. The optimization
; should now handle the reduction inside the condition.

; CHECK: IR Dump After HIRMemoryReductionSinkingPass
; CHECK: BEGIN REGION { modified }
; CHECK:       if (0 < smin(99, (-1 + %d)) + 1)
; CHECK:       {
; CHECK:          %limm = (%t13)[5];
; CHECK:             %tmp = 0;
; CHECK:          + DO i1 = 0, smin(99, (-1 + %d)), 1   <DO_LOOP>
; CHECK:          |   %0 = (%t14)[i1];
; CHECK:          |   %t15 = %limm;
; CHECK:          |   %limm = %t15 + %m;
; CHECK:          |   if (%0 >u 112)
; CHECK:          |   {
; CHECK:          |      %tmp = %tmp  +  %0;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:          if (%tmp != 0)
; CHECK:          {
; CHECK:             %t16 = (%t13)[7];
; CHECK:             (%t13)[7] = %t16 + %tmp;
; CHECK:          }
; CHECK:          (%t13)[5] = %limm;
; CHECK:       }
; CHECK:       if (smax(0, %d) < smin(99, %d) + 1)
; CHECK:       {
; CHECK:          %0 = (%t14)[smax(0, %d)];
; CHECK:          if (%0 >u 112)
; CHECK:          {
; CHECK:             %t16 = (%t13)[7];
; CHECK:             (%t13)[7] = %0 + %t16;
; CHECK:          }
; CHECK:       }
; CHECK:       if (smax(0, (1 + %d)) < 100)
; CHECK:       {
; CHECK:          %limm9 = (%t13)[5];
; CHECK:             %tmp12 = 0;
; CHECK:          + DO i1 = 0, -1 * smax(0, (1 + %d)) + 99, 1   <DO_LOOP>
; CHECK:          |   %0 = (%t14)[i1 + smax(0, (1 + %d))];
; CHECK:          |   %t15 = %limm9;
; CHECK:          |   %limm9 = %t15 + %m;
; CHECK:          |   if (%0 >u 112)
; CHECK:          |   {
; CHECK:          |      %tmp12 = %tmp12  +  %0;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:          if (%tmp12 != 0)
; CHECK:          {
; CHECK:             %t16 = (%t13)[7];
; CHECK:             (%t13)[7] = %t16 + %tmp12;
; CHECK:          }
; CHECK:          (%t13)[5] = %limm9;
; CHECK:       }
; CHECK: END REGION


define void @foo(ptr noalias %t13,ptr noalias %t14, i64 %m, i64 %d) {
entry:
  br label %for.body56

for.body56:                                       ; preds = %for.body56, %entry
  %indvars.iv158 = phi i64 [ 0, %entry ], [ %indvars.iv.next159, %if.next.end ]
  %arrayidx58 = getelementptr inbounds i64, ptr %t13, i64 5
  %arrayidx = getelementptr inbounds i64, ptr %t14, i64 %indvars.iv158
  %0 = load i64, ptr %arrayidx
  %cmp1 = icmp ne i64 %indvars.iv158, %d
  br i1 %cmp1, label %if.then, label %if.end

if.then:
  %t15 = load i64, ptr %arrayidx58, align 4
  %add = add nuw nsw i64 %m, %t15
  store i64 %add, ptr %arrayidx58, align 4
  br label %if.end

if.end:
  %cmp2 = icmp ugt i64 %0, 112
  br i1 %cmp2, label %if.next.then, label %if.next.end

if.next.then:
  %arrayidx3 = getelementptr inbounds i64, ptr %t13, i64 7
  %t16 = load i64, ptr %arrayidx3, align 4
  %1 = add nuw nsw i64 %0, %t16
  store i64 %1, ptr %arrayidx3, align 4
  br label %if.next.end

if.next.end:
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, 100
  br i1 %exitcond161, label %for.end62.loopexit, label %for.body56

for.end62.loopexit:
  ret void
}