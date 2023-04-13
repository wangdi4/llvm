; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Src code-

; for (a=0; a<n; a++)
;  for (b=0; b<n; b++)
;   for (c=0; c<n; c++)
;    for (d=0; d<n; d++)
;     for (e=0; e<n; e++)
;      for (f=0; f<n; f++)
;       x++;


; Check parsing output for the loop verifying that %t2 is parsed in terms of
; %x.267.out. The SCEV of %t2 is an AddRec in terms of %x.267. Propagating this
; information outside the loop leads to live range violation as %x.267 is
; updated in i3 loop so we create liveout copy of %x.267.

; CHECK: + DO i1 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK: |   %x.171 = %x.075;
; CHECK: |
; CHECK: |   + DO i2 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK: |   |   %x.267 = %x.171;
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK: |   |   |   %x.267.out = %x.267;
; CHECK: |   |   |
; CHECK: |   |   |   + DO i4 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK: |   |   |   |   %t2 = (%cond84 * %cond84)  +  (%cond84 * %cond84) * i4 + %x.267.out;
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   |
; CHECK: |   |   |   %x.267 = %x.267.out + (%cond84 * %cond84 * %cond84);
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   %x.171 = %x.267.out + (%cond84 * %cond84 * %cond84);
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %x.075 = %x.267.out + (%cond84 * %cond84 * %cond84);
; CHECK: + END LOOP

; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-details -disable-output  2>&1 | FileCheck %s -check-prefix=DETAIL

; Verify loop liveins/liveouts.

; DETAIL: LiveIn symbases: [[X075SYMBASE:[0-9]+]], [[COND84SYMBASE:[0-9]+]]
; DETAIL: LiveOut symbases: [[SYMBASE2:[0-9]+]]
; DETAIL: DO i32 i1
; DETAIL: <BLOB> LINEAR i32 %cond84 {sb:[[COND84SYMBASE]]
; DETAIL: <BLOB> NON-LINEAR i32 %x.075 {sb:[[X075SYMBASE]]

; DETAIL: LiveIn symbases: [[COND84SYMBASE]], [[X171SYMBASE:[0-9]+]]
; DETAIL: LiveOut symbases: [[SYMBASE2]]
; DETAIL: DO i32 i2
; DETAIL: <BLOB> NON-LINEAR i32 %x.171 {sb:[[X171SYMBASE]]

; DETAIL: LiveIn symbases: [[COND84SYMBASE]], [[X267SYMBASE:[0-9]+]]
; DETAIL: LiveOut symbases: [[SYMBASE2]]
; DETAIL: DO i32 i3

; DETAIL: <LVAL-REG> NON-LINEAR i32 %x.267.out {sb:[[X267OUTSYMBASE:[0-9]+]]}
; DETAIL: LiveIn symbases: [[COND84SYMBASE]], [[X267OUTSYMBASE]]
; DETAIL: LiveOut symbases: [[SYMBASE2]]
; DETAIL: DO i32 i4
; DETAIL: <BLOB> LINEAR i32 %x.267.out{def@3} {sb:[[X267OUTSYMBASE]]}

; DETAIL: <BLOB> NON-LINEAR i32 %x.267.out {sb:[[X267OUTSYMBASE]]}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main(i32 %cond84) {
entry:
  br label %for.cond2.preheader.lr.ph

for.cond2.preheader.lr.ph:                        ; preds = %cond.end, %entry
  %t1 = mul i32 %cond84, %cond84
  br label %for.cond5.preheader.preheader

for.cond5.preheader.preheader:                    ; preds = %for.inc30, %for.cond2.preheader.lr.ph
  %x.075 = phi i32 [ 0, %for.cond2.preheader.lr.ph ], [ %.lcssa.lcssa.lcssa, %for.inc30 ]
  %a.074 = phi i32 [ 0, %for.cond2.preheader.lr.ph ], [ %inc31, %for.inc30 ]
  br label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.inc27, %for.cond5.preheader.preheader
  %x.171 = phi i32 [ %.lcssa.lcssa, %for.inc27 ], [ %x.075, %for.cond5.preheader.preheader ]
  %b.070 = phi i32 [ %inc28, %for.inc27 ], [ 0, %for.cond5.preheader.preheader ]
  br label %for.cond11.preheader.preheader

for.cond11.preheader.preheader:                   ; preds = %for.inc24, %for.cond8.preheader.preheader
  %x.267 = phi i32 [ %.lcssa, %for.inc24 ], [ %x.171, %for.cond8.preheader.preheader ]
  %c.066 = phi i32 [ %inc25, %for.inc24 ], [ 0, %for.cond8.preheader.preheader ]
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21, %for.cond11.preheader.preheader
  %x.363 = phi i32 [ %t2, %for.inc21 ], [ %x.267, %for.cond11.preheader.preheader ]
  %d.062 = phi i32 [ %inc22, %for.inc21 ], [ 0, %for.cond11.preheader.preheader ]
  %t2 = add i32 %t1, %x.363
  %inc22 = add nuw nsw i32 %d.062, 1
  %exitcond78 = icmp eq i32 %inc22, %cond84
  br i1 %exitcond78, label %for.inc24, label %for.inc21

for.inc24:                                        ; preds = %for.inc21
  %.lcssa = phi i32 [ %t2, %for.inc21 ]
  %inc25 = add nuw nsw i32 %c.066, 1
  %exitcond79 = icmp eq i32 %inc25, %cond84
  br i1 %exitcond79, label %for.inc27, label %for.cond11.preheader.preheader

for.inc27:                                        ; preds = %for.inc24
  %.lcssa.lcssa = phi i32 [ %.lcssa, %for.inc24 ]
  %inc28 = add nuw nsw i32 %b.070, 1
  %exitcond80 = icmp eq i32 %inc28, %cond84
  br i1 %exitcond80, label %for.inc30, label %for.cond8.preheader.preheader

for.inc30:                                        ; preds = %for.inc27
  %.lcssa.lcssa.lcssa = phi i32 [ %.lcssa.lcssa, %for.inc27 ]
  %inc31 = add nuw nsw i32 %a.074, 1
  %exitcond81 = icmp eq i32 %inc31, %cond84
  br i1 %exitcond81, label %for.end32.loopexit, label %for.cond5.preheader.preheader

for.end32.loopexit:                               ; preds = %for.inc30
  %.lcssa.lcssa.lcssa.lcssa = phi i32 [ %.lcssa.lcssa.lcssa, %for.inc30 ]
  br label %for.end32

for.end32:                                        ; preds = %for.end32.loopexit, %cond.end
  %x.0.lcssa = phi i32 [ %.lcssa.lcssa.lcssa.lcssa, %for.end32.loopexit ]
  ret i32 0
}

