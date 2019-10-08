; RUN: opt -hir-framework -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-hcfg -disable-output -vplan-force-vf=2 < %s 2>&1 | FileCheck %s

; Verify that VPlan H-CFG builder is able to build the H-CFG for a simple HIR
; multi-exit loop.

; Input Loop:
; <19>      + DO i1 = 0, %len_limit + -2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4294967295>
; <9>       |   if ((%cur)[i1 + -1 * zext.i32.i64(%delta2) + 1] != (%cur)[i1 + 1])
; <9>       |   {
; <20>      |      %len_best.011.out = i1 + 1;
; <11>      |      goto for.end.loopexit;
; <9>       |   }
; <19>      + END LOOP
; <2>          %len_best.011.out = %len_limit + -1;

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @peel_example(i32 %delta2, i32 %len_limit, i32* nocapture readonly %cur) local_unnamed_addr #0 {
; CHECK-LABEL:  Print after building H-CFG:
; CHECK-NEXT:    REGION: [[REGION0:region[0-9]+]] (BP: NULL)
; CHECK-NEXT:    [[BB0:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[LOOP0:loop[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    REGION: [[LOOP0]] (BP: NULL)
; CHECK-NEXT:    [[BB1:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     [DA: Uniform]   i32 [[VP0:%.*]] = add i32 [[LEN_LIMIT0:%.*]] i32 -2
; CHECK-NEXT:    SUCCESSORS(1):[[BB2:BB[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]] (BP: NULL) :
; CHECK-NEXT:     [DA: Divergent] i32 [[VP1:%.*]] = phi  [ i32 0, [[BB1]] ],  [ i32 [[VP2:%.*]], [[BB3:BB[0-9]+]] ]
; CHECK-NEXT:     [DA: Uniform]   i64 [[VP3:%.*]] = zext i32 [[DELTA20:%.*]] to i64
; CHECK-NEXT:     [DA: Uniform]   i64 [[VP4:%.*]] = mul i64 [[VP3]] i64 -1
; CHECK-NEXT:     [DA: Divergent] i64 [[VP5:%.*]] = zext i32 [[VP1]] to i64
; CHECK-NEXT:     [DA: Divergent] i64 [[VP6:%.*]] = add i64 [[VP4]] i64 [[VP5]]
; CHECK-NEXT:     [DA: Divergent] i64 [[VP7:%.*]] = add i64 [[VP6]] i64 1
; CHECK-NEXT:     [DA: Divergent] i32* [[VP8:%.*]] = getelementptr inbounds i32* [[CUR0:%.*]] i64 [[VP7]]
; CHECK-NEXT:     [DA: Divergent] i32 [[VP9:%.*]] = load i32* [[VP8]]
; CHECK-NEXT:     [DA: Divergent] i32 [[VP10:%.*]] = add i32 [[VP1]] i32 1
; CHECK-NEXT:     [DA: Divergent] i64 [[VP11:%.*]] = zext i32 [[VP10]] to i64
; CHECK-NEXT:     [DA: Divergent] i32* [[VP12:%.*]] = getelementptr inbounds i32* [[CUR0]] i64 [[VP11]]
; CHECK-NEXT:     [DA: Divergent] i32 [[VP13:%.*]] = load i32* [[VP12]]
; CHECK-NEXT:     [DA: Divergent] i1 [[VP14:%.*]] = icmp i32 [[VP9]] i32 [[VP13]]
; CHECK-NEXT:    SUCCESSORS(1):[[BB4:BB[0-9]+]]
; CHECK-NEXT:    PREDECESSORS(2): [[BB1]] [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB4]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition([[BB2]]): [DA: Divergent] i1 [[VP14]] = icmp i32 [[VP9]] i32 [[VP13]]
; CHECK-NEXT:    SUCCESSORS(2):[[BB5:BB[0-9]+]](i1 [[VP14]]), [[BB3]](!i1 [[VP14]])
; CHECK-NEXT:    PREDECESSORS(1): [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]] (BP: NULL) :
; CHECK-NEXT:       [DA: Divergent] i32 [[VP2]] = add i32 [[VP1]] i32 1
; CHECK-NEXT:       [DA: Divergent] i1 [[VP15:%.*]] = icmp i32 [[VP2]] i32 [[VP0]]
; CHECK-NEXT:      SUCCESSORS(2):[[BB2]](i1 [[VP15]]), [[BB6:BB[0-9]+]](!i1 [[VP15]])
; CHECK-NEXT:      PREDECESSORS(1): [[BB4]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB6]] (BP: NULL) :
; CHECK-NEXT:       <Empty Block>
; CHECK-NEXT:      SUCCESSORS(1):[[BB7:BB[0-9]+]]
; CHECK-NEXT:      PREDECESSORS(1): [[BB3]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB5]] (BP: NULL) :
; CHECK-NEXT:       [DA: Uniform]   br for.end.loopexit
; CHECK-NEXT:      SUCCESSORS(1):[[BB7]]
; CHECK-NEXT:      PREDECESSORS(1): [[BB4]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB7]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(2): [[BB5]] [[BB6]]
; CHECK-EMPTY:
; CHECK-NEXT:    SUCCESSORS(1):[[BB8:BB[0-9]+]]
; CHECK-NEXT:    END Region([[LOOP0]])
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB8]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): [[LOOP0]]
; CHECK-EMPTY:
; CHECK-NEXT:    END Region([[REGION0]])
;
entry:
  %cmp10 = icmp eq i32 %len_limit, 1
  br i1 %cmp10, label %for.end, label %for.body.lr.ph

for.body.lr.ph:
  %idx.ext1 = zext i32 %delta2 to i64
  %idx.neg = sub nsw i64 0, %idx.ext1
  br label %for.body

for.body:
  %len_best.011 = phi i32 [ 1, %for.body.lr.ph ], [ %inc, %for.inc ]
  %idx.ext = zext i32 %len_best.011 to i64
  %add.ptr = getelementptr inbounds i32, i32* %cur, i64 %idx.ext
  %add.ptr2 = getelementptr inbounds i32, i32* %add.ptr, i64 %idx.neg
  %0 = load i32, i32* %add.ptr2
  %1 = load i32, i32* %add.ptr
  %cmp3 = icmp eq i32 %0, %1
  br i1 %cmp3, label %for.inc, label %for.end.loopexit

for.inc:
  %inc = add i32 %len_best.011, 1
  %cmp = icmp eq i32 %inc, %len_limit
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:
  %len_best.0.lcssa.ph = phi i32 [ %len_limit, %for.inc ], [ %len_best.011, %for.body ]
  br label %for.end

for.end:
  %len_best.0.lcssa = phi i32 [ 1, %entry ], [ %len_best.0.lcssa.ph, %for.end.loopexit ]
  ret i32 %len_best.0.lcssa
}
