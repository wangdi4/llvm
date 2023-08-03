; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that a negative 64 bit divisor of udiv is converted into a blob form.
; We parse it like this because negative denominators are not allowed in CanonExpr.
; CanonExpr has logic to automatically negate the numerator when a negative
; denominator is provided but that is incorrect for unsigned division. That will
; be fixed separately.

; Previous incorrect HIR had this form where we negated the denominator-
; + DO i1 = 0, 127, 1   <DO_LOOP>
; |   (%arr)[i1] = (i1 + -3)/2;
; + END LOOP

; CHECK: + DO i1 = 0, 127, 1   <DO_LOOP>
; CHECK: |   %add = i1  +  -3;
; CHECK: |   (%arr)[i1] = (%add /u -2);
; CHECK: + END LOOP

define dso_local void @foo(ptr nocapture %arr) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %add = add i64 %indvars.iv, -3
  %div = udiv i64 %add, -2
  %arrayidx = getelementptr inbounds i64, ptr %arr, i64 %indvars.iv
  store i64 %div, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

