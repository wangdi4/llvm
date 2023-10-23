; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,hir-vec-dir-insert,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we distribute the select reduction chain into a separate loop
; because vectorizer doesn't handle it.

; + DO i1 = 0, 4, 1   <DO_LOOP>
; |   %sel.1 = (%val1 > %sel.phi) ? %val1 : %sel.phi;
; |   %ld1 = (%in1)[i1];
; |   %sel.2 = (%val2 > %sel.1) ? %val2 : %sel.1;
; |   %ld2 = (%in2)[i1];
; |   %sel.phi = (%val3 > %sel.2) ? %val3 : %sel.2;
; |   %ld3 = (%in3)[i1];
; |   (%out)[i1] = %ld1 + %ld2 + %ld3;
; + END LOOP

; CHECK: BEGIN REGION { modified }

; CHECK-NOT: llvm.directive.region.entry()

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %sel.1 = (%val1 > %sel.phi) ? %val1 : %sel.phi; <Safe Reduction>
; CHECK: |   %sel.2 = (%val2 > %sel.1) ? %val2 : %sel.1; <Safe Reduction>
; CHECK: |   %sel.phi = (%val3 > %sel.2) ? %val3 : %sel.2; <Safe Reduction>
; CHECK: + END LOOP

; CHECK: %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %ld1 = (%in1)[i1];
; CHECK: |   %ld2 = (%in2)[i1];
; CHECK: |   %ld3 = (%in3)[i1];
; CHECK: |   (%out)[i1] = %ld1 + %ld2 + %ld3;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

define void @foo(float %s, float %val1, float %val2, float %val3, ptr noalias %in1, ptr noalias %in2, ptr noalias %in3, ptr noalias %out) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %sel.phi = phi float [ %s, %entry ], [ %sel.3, %loop ]
  %cmp.1 = fcmp fast ogt float %val1, %sel.phi
  %sel.1 = select i1 %cmp.1, float %val1, float %sel.phi
  %gep1 = getelementptr i32, ptr %in1, i64 %iv
  %ld1 = load i32, ptr %gep1
  %cmp.2 = fcmp fast ogt float %val2, %sel.1
  %sel.2 = select i1 %cmp.2, float %val2, float %sel.1
  %gep2 = getelementptr i32, ptr %in2, i64 %iv
  %ld2 = load i32, ptr %gep2
  %cmp.3 = fcmp fast ogt float %val3, %sel.2
  %sel.3 = select i1 %cmp.3, float %val3, float %sel.2
  %gep3 = getelementptr i32, ptr %in3, i64 %iv
  %ld3 = load i32, ptr %gep3
  %add1 = add i32 %ld1, %ld2
  %add2 = add i32 %add1, %ld3
  %gep4 = getelementptr i32, ptr %out, i64 %iv
  store i32 %add2, ptr %gep4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv, 4
  br i1 %cmp, label %exit, label %loop

exit:
  %lcssa = phi float [ %sel.3, %loop ]
  ret void
}
