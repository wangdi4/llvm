; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

;
; Test vectorization of an unmasked store of a divergent value to a uniform
; memory location. The divergent value being stored in this test can be:
;   - The loop IV - in which case we can use IV + VF - 1 as the value being
;     stored.
;   - Something other than the IV - in which we can use extracted value from
;     the last vector lane as the value being stored.
;
; CHECK:             + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:        |   %.vec = (<4 x i64>*)(%larr)[i1];
; CHECK-NEXT:        |   %extract.3. = extractelement %.vec,  3;
; CHECK-NEXT:        |   (%unif_p_divval)[0] = %extract.3.;
; CHECK-NEXT:        |   (%unif_p_iv)[0] = i1 + 3;
; CHECK-NEXT:        |   (<4 x i64>*)(%larr)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:        + END LOOP
;
define void @foo(i64* noalias nocapture %unif_p_iv, i64* noalias nocapture %unif_p_divval, i64* noalias nocapture %larr) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.07 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %l1.07
  %larrval = load i64, i64* %arrayidx, align 8
  store i64 %larrval, i64* %unif_p_divval, align 8
  store i64 %l1.07, i64* %unif_p_iv, align 8
  store i64 %l1.07, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.07, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
