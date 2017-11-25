; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Verify that NSW is not applied to this multi-exit loop as the range information for the IV is computed off of the early exit by scalar evolution. 
; %i.024.i66183 has a range of [5, 8). 
; We cannot use a signed comparison for the bottom test as the upper bound may be signed negative number.

; HIR-
; + DO i1 = 0, %n + -6, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3>
; |   %r.025.i65182.out = %r.025.i65182;
; |   %r.025.i65182 = (i1 + 5 == 5) ? %r.025.i65182.out + 5 : %r.025.i65182;
; |   if (i1 + 5 >= 7)
; |   {
; |      goto foo.exit80;
; |   }
; + END LOOP

; CHECK: NSW: No

define void @foo(i32 %add4.i70, i32 %n) {
entry:
  br label %for.inc.i79

for.inc.i79:                                      ; preds = %for.body.i67, %entry
  %i.024.i66183 = phi i32 [ 5, %entry ], [ %inc.i77, %for.body.i67 ]
  %r.025.i65182 = phi i32 [ %add4.i70, %entry ], [ %add8.r.2.i74, %for.body.i67 ]
  %cmp6.i72 = icmp eq i32 %i.024.i66183, 5
  %add8.i73 = add nsw i32 %r.025.i65182, 5
  %add8.r.2.i74 = select i1 %cmp6.i72, i32 %add8.i73, i32 %r.025.i65182
  %inc.i77 = add nuw nsw i32 %i.024.i66183, 1
  %cmp.i78 = icmp slt i32 %i.024.i66183, 7
  br i1 %cmp.i78, label %for.body.i67, label %foo.exit80

for.body.i67:                                     ; preds = %for.inc.i79
  %cond197 = icmp eq i32 %inc.i77, %n
  br i1 %cond197, label %foo.exit80, label %for.inc.i79

foo.exit80:
  %r.lcssa = phi i32 [ %r.025.i65182, %for.inc.i79 ], [ %r.025.i65182, %for.body.i67 ]
  ret void
}

