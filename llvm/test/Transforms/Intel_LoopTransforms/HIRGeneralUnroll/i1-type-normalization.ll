; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that we are succesfully able to normalize i1 type subscript of (%A)[-1 * i1].
; The truncation to i1 of lower blob (8 * tgu) is simplified to zero by
; ScalarEvolution so normalization is successful here.

; Incoming HIR-
;  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;  |   (%A)[-1 * i1] = 3;
;  + END LOOP

; CHECK: %tgu = (%n)/u8;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP> <nounroll>
; CHECK: |   (%A)[-8 * i1] = 3;
; CHECK: |   (%A)[-8 * i1 + -1] = 3;
; CHECK: |   (%A)[-8 * i1 + -2] = 3;
; CHECK: |   (%A)[-8 * i1 + -3] = 3;
; CHECK: |   (%A)[-8 * i1 + -4] = 3;
; CHECK: |   (%A)[-8 * i1 + -5] = 3;
; CHECK: |   (%A)[-8 * i1 + -6] = 3;
; CHECK: |   (%A)[-8 * i1 + -7] = 3;
; CHECK: + END LOOP

; CHECK: switch(%n + -8 * %tgu + -1)
; CHECK: {
; CHECK: case 6:
; CHECK:   L6.26:
; CHECK:   (%A)[0] = 3;
; CHECK:   goto L5.28;
; CHECK: case 5:
; CHECK:   L5.28:
; CHECK:   (%A)[1] = 3;
; CHECK:   goto L4.31;
; CHECK: case 4:
; CHECK:   L4.31:
; CHECK:   (%A)[0] = 3;
; CHECK:   goto L3.34;
; CHECK: case 3:
; CHECK:   L3.34:
; CHECK:   (%A)[1] = 3;
; CHECK:   goto L2.37;
; CHECK: case 2:
; CHECK:   L2.37:
; CHECK:   (%A)[0] = 3;
; CHECK:   goto L1.40;
; CHECK: case 1:
; CHECK:   L1.40:
; CHECK:   (%A)[1] = 3;
; CHECK:   goto L0.43;
; CHECK: case 0:
; CHECK:   L0.43:
; CHECK:   (%A)[0] = 3;
; CHECK:   break;
; CHECK: default:
; CHECK:   break;
; CHECK: }

define void @foo(ptr %A, i64 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %and = and i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %and
  store i32 3, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

