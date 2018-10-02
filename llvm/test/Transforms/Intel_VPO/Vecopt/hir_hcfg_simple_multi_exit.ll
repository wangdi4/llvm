; RUN: opt -hir-framework -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-after-hcfg -disable-output -vplan-force-vf=1 < %s 2>&1 | FileCheck %s

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

; VPlan Output:
; REGION: region1 (BP: NULL)
; BB2 (BP: NULL) :
;  <Empty Block>
; SUCCESSORS(1):loop11
;
; REGION: loop11 (BP: NULL)
; BB3 (BP: NULL) :
;  %vp12304 = add %vp12176 i32 -2
; SUCCESSORS(1):BB5
;
; BB5 (BP: NULL) :
;  %vp1296 = semi-phi i32 0 %vp11920
;  %vp5184 = load %vp46704
;  %vp6432 = load %vp6368
;  %vp6576 = icmp %vp5184 %vp6432
; SUCCESSORS(1):BB10
;
; BB10 (BP: NULL) :
;  <Empty Block>
;  Condition(BB5): %vp6576 = icmp %vp5184 %vp6432
; SUCCESSORS(2):BB6(%vp6576), BB7(!%vp6576)
;
;   BB7 (BP: NULL) :
;    %vp11920 = add %vp1296 i32 1
;    %vp12448 = icmp %vp11920 %vp12304
;   SUCCESSORS(2):BB5(%vp12448), BB8(!%vp12448)
;
;   BB8 (BP: NULL) :
;    <Empty Block>
;   SUCCESSORS(1):BB4
;
;   BB6 (BP: NULL) :
;    <Empty Block>
;   SUCCESSORS(1):BB4
;
; BB4 (BP: NULL) :
;  <Empty Block>
; END Block - no SUCCESSORS
; SUCCESSORS(1):BB9
; END Region(loop11)
;
; BB9 (BP: NULL) :
;  <Empty Block>
; END Block - no SUCCESSORS
; END Region(region1)


; CHECK: REGION: loop{{[0-9]+}}

; Loop PH.
; CHECK: SUCCESSORS(1):[[H:BB[0-9]+]]

; Loop H.
; CHECK: [[H]] (BP: NULL) :

; Early exit condition.
; CHECK: SUCCESSORS(2):[[EEXIT:BB[0-9]+]](%vp{{[0-9]+}}), [[LATCH:BB[0-9]+]]

; Latch condition.
; CHECK: SUCCESSORS(2):[[H]](%vp{{[0-9]+}}), [[REG_EXIT:BB[0-9]+]]

; Regular exit going to landing pad.
; CHECK: [[REG_EXIT]] (BP: NULL)
; CHECK: SUCCESSORS(1):[[LANDING_PAD:BB[0-9]+]]

; Early exit going to landing pad.
; CHECK: [[EEXIT]] (BP: NULL)
; CHECK: SUCCESSORS(1):[[LANDING_PAD:BB[0-9]+]]

; Landing pad.
; CHECK: [[LANDING_PAD]] (BP: NULL)
; CHECK: END Block - no SUCCESSORS

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @peel_example(i32 %delta2, i32 %len_limit, i32* nocapture readonly %cur) local_unnamed_addr #0 {
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
