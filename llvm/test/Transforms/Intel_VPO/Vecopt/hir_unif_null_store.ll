;
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
;
; LIT test to check that we do not crash when we have a uniform memory
; access to null[0]. IPO multi versioning is creating such loops and
; we are crashing during HIR vector code generation.
;
; Incoming HIR:
;
;     + DO i1 = 0, 1023, 1   <DO_LOOP>
;     |   (null)[0] = 0
;     + END LOOP
;
; CHECK:          + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:     |   %nsbgepcopy = null
; CHECK-NEXT:     |   (%nsbgepcopy)[0] = 0
; CHECK-NEXT:     + END LOOP
;
define void @foo() {
entry:
  br label %for.body

for.body:
  %l1.05 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  store i64 0, ptr null, align 8
  %inc = add nuw nsw i64 %l1.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  ret void
}
