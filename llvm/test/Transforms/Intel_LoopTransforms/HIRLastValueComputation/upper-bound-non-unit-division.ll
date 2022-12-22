; When loop upper bound has non-unit division, we need to convert upper bound to a stand-alone blob
; before replacing IV with upper bound.
; Without converting upper bound to a stand-alone blob, it shows %add2 = (-1 * %a + 7 * %c + 41)/u7  +  %b
; The result would be wrong if the numerator becomes negative.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Last Value Computation ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<10>               + DO i1 = 0, (-1 * %a + 41)/u7, 1   <DO_LOOP>  <MAX_TC_EST = 613566757>
;<3>                |   %add2 = i1 + %c  +  %b;
;<10>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;Function: foo
;
; CHECK:     BEGIN REGION { modified }
; CHECK:           %add2 = %c + ((41 + (-1 * %a)) /u 7)  +  %b;
; CHECK:      END REGION
;
; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local i32 @foo(i32 %a, i32 %b, i32 %c) local_unnamed_addr {
entry:
  %add = sub i32 41, %a
  %div = udiv i32 %add, 7
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %t.0.lcssa = phi i32 [ %add2, %for.body ]
  ret i32 %t.0.lcssa

for.body:                                         ; preds = %entry, %for.body
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add1 = add i32 %i.01, %c
  %add2 = add i32 %add1, %b
  %inc = add nuw nsw i32 %i.01, 1
  %cmp = icmp ugt i32 %inc, %div
  br i1 %cmp, label %for.cond.cleanup, label %for.body
}
