; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to show an issue with folding of add operation into the canon
; expression when the canon expression has a denominator. When we do the
; add folding with denominator(s), the add operation effectively does
; the following:
;
;     CE1/D1 + CE2/D2 ==> ((LCM/D1 * CE1) + (LCM/D2 * CE2))/LCM
;
; This cannot be done blindly as we need to make sure that no overflow
; occurs.
;
; Scalar HIR:
;              + DO i1 = 0, 1023, 1   <DO_LOOP>
;              |   %div1 = i1 + %n1  /  2;
;              |   (%lp)[i1] = %div1 + 1;
;              + END LOOP
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; FIXME:  Doing the add (i1 + %n1 + 2) before the divide can overflow leading
; to incorrect results from vectorization.
; CHECK-NEXT:      |   (<4 x i64>*)(%lp)[i1] = (i1 + %n1 + <i64 0, i64 1, i64 2, i64 3> + 2)/2;
; CHECK-NEXT:      + END LOOP
;
define void @foo(i64* %lp, i64 %n1) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.08 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nsw i64 %l1.08, %n1
  %div1 = sdiv i64 %add, 2
  %add2 = add nsw i64 %div1, 1
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.08
  store i64 %add2, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.08, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
