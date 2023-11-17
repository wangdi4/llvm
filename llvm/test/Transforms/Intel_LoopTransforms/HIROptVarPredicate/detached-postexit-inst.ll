; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate" -print-before=hir-opt-var-predicate -print-after=hir-opt-var-predicate -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Verify that we don't fail in removeRedundantNodes() after handling the
; condition if (i1 != 0). The issue was that the i3 loop is dead because its
; ztt is false. removeRedundantNodes() removes the loop along with its
; postexit but the visitor still tries to visit the detached postexit inst
; and fails when invoking getLexicalParentLoop() on it.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 32, 1   <DO_LOOP>
; CHECK: |   if (i1 != 0)
; CHECK: |   {
; CHECK: |      (null)[0] = 0.000000e+00;
; CHECK: |   }
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   %sum.0.lcssa.in = 0.000000e+00;
; CHECK: |   |
; CHECK: |   |   + Ztt: if (undef false undef)

; CHECK: |   |   + DO i64 i3 = 0, %indvars.iv463, 1   <DO_LOOP>
; CHECK: |   |   |   %add498.lcssa.in = 0.000000e+00;
; CHECK: |   |   + END LOOP
; CHECK: |   |      %sum.0.lcssa.in = 0.000000e+00;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK:      + DO i64 i1 = 0, 31, 1   <DO_LOOP>
; CHECK-NEXT: |   (null)[0] = 0.000000e+00;

; CHECK-NOT: DO

; CHECK:      + END LOOP


define void @_Z12init_wfnvlocv(i64 %indvars.iv463) {
_ZNSt6vectorISt5arrayIfLm3EESaIS1_EE17_S_check_init_lenEmRKS2_.exit.i:
  br label %for.cond391.preheader

for.cond391.preheader:                            ; preds = %for.cond.cleanup468, %_ZNSt6vectorISt5arrayIfLm3EESaIS1_EE17_S_check_init_lenEmRKS2_.exit.i
  %indvars.iv4631 = phi i64 [ 0, %_ZNSt6vectorISt5arrayIfLm3EESaIS1_EE17_S_check_init_lenEmRKS2_.exit.i ], [ %indvars.iv.next464, %for.cond.cleanup468 ]
  %cmp392351.not = icmp eq i64 %indvars.iv4631, 0
  br i1 %cmp392351.not, label %for.cond406.preheader, label %for.body394.preheader

for.body394.preheader:                            ; preds = %for.cond391.preheader
  store float 0.000000e+00, ptr null, align 4
  br label %for.cond406.preheader

for.cond406.preheader:                            ; preds = %for.body394.preheader, %for.cond391.preheader
  br label %for.cond471.preheader

for.cond471.preheader:                            ; preds = %for.cond.cleanup487, %for.cond406.preheader
  br i1 true, label %for.cond.cleanup487, label %for.body488.lr.ph

for.cond.cleanup468:                              ; preds = %for.cond.cleanup487
  %indvars.iv.next464 = add i64 %indvars.iv4631, 1
  %exitcond465.not = icmp eq i64 %indvars.iv4631, 32
  br i1 %exitcond465.not, label %_ZNSt6vectorISt5arrayIfLm3EESaIS1_EED2Ev.exit, label %for.cond391.preheader

for.body488.lr.ph:                                ; preds = %for.cond471.preheader
  br label %for.body488

for.cond.cleanup487.loopexit:                     ; preds = %for.body488
  %add498.lcssa = phi float [ 0.000000e+00, %for.body488 ]
  br label %for.cond.cleanup487

for.cond.cleanup487:                              ; preds = %for.cond.cleanup487.loopexit, %for.cond471.preheader
  %sum.0.lcssa = phi float [ 0.000000e+00, %for.cond471.preheader ], [ 0.000000e+00, %for.cond.cleanup487.loopexit ]
  %exitcond450.not = icmp eq i64 0, 0
  br i1 %exitcond450.not, label %for.cond.cleanup468, label %for.cond471.preheader

for.body488:                                      ; preds = %for.body488, %for.body488.lr.ph
  %indvars.iv441 = phi i64 [ 0, %for.body488.lr.ph ], [ %indvars.iv.next442, %for.body488 ]
  %indvars.iv.next442 = add i64 %indvars.iv441, 1
  %exitcond444.not = icmp eq i64 %indvars.iv441, %indvars.iv463
  br i1 %exitcond444.not, label %for.cond.cleanup487.loopexit, label %for.body488

_ZNSt6vectorISt5arrayIfLm3EESaIS1_EED2Ev.exit:    ; preds = %for.cond.cleanup468
  ret void
}
