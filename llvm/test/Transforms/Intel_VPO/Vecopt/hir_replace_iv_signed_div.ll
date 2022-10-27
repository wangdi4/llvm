; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec' -print-before=hir-vplan-vec -print-after=hir-vplan-vec -vplan-vec-scenario="n0;v4;m4" %s 2>&1 | FileCheck %s

; Verify that we can successfully generate unrolled masked remainder loop in
; this case. The utility replaceIVByCanonExpr() was choking on this vector ref
; with signed division by 100-
; (%t + i1 + <i64 0, i64 1, i64 2, i64 3>)/100

; This test is expected to fail until vectorizer checks in PR to unroll
; masked remainder loop.

; XFAIL: *

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 6, 1   <DO_LOOP>
; CHECK: |   %div = i1 + %t  /  100;
; CHECK: |   (%A)[i1] = %div;
; CHECK: + END LOOP

; CHECK: Dump After

; Unrolled main Loop

; CHECK: (<4 x i64>*)(%A)[0] = (%t + <i64 0, i64 1, i64 2, i64 3>)/100;
; CHECK: %phi.temp = 4;


; Unrolled masked remainder loop

; CHECK: %.vec6 = %phi.temp + <i64 0, i64 1, i64 2, i64 3> <u 7;

; Verify that replacement of IV by lower is successful when CE has signed division by 100

; CHECK: (<4 x i64>*)(%A)[%phi.temp] = (%t + %phi.temp + <i64 0, i64 1, i64 2, i64 3>)/100, Mask = @{%.vec6};


define void @foo(i64 *%A, i64 %t) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %gep = getelementptr inbounds i64, i64* %A, i64 %iv
  %add = add i64 %iv, %t
  %div = sdiv i64 %add, 100
  store i64 %div, i64* %gep
  %iv.inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 7
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
