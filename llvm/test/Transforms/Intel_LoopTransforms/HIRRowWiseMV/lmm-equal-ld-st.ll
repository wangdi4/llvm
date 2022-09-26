; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-rowwise-mv -print-before=hir-rowwise-mv -print-after=hir-rowwise-mv -hir-rowwise-mv-skip-dtrans -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-rowwise-mv,print<hir>" -aa-pipeline="basic-aa" -hir-rowwise-mv-skip-dtrans -disable-output 2>&1 < %s | FileCheck %s

; This test checks that HIRLMM-specific assumptions don't lead to equal stores
; being ignored in HIRRowWiseMV invariance checks.

; Print before:

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK-NEXT:       |   + DO i2 = 0, 31, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   %bj.next = (%b)[i2]  +  (%A)[32 * i1 + i2];
; CHECK-NEXT:       |   |   (%b)[i2] = %bj.next;
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; Print after: (no change)

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK-NEXT:       |   + DO i2 = 0, 31, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   %bj.next = (%b)[i2]  +  (%A)[32 * i1 + i2];
; CHECK-NEXT:       |   |   (%b)[i2] = %bj.next;
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

define void @lmm-equal-ld-st(double* %A, double* %b) #0 {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  %A_row = mul nuw nsw i64 %i, 32
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2 ]
  %A_ind = add nuw nsw i64 %A_row, %j
  %Aijp = getelementptr inbounds double, double* %A, i64 %A_ind
  %Aij = load double, double* %Aijp
  %bjp = getelementptr inbounds double, double* %b, i64 %j
  %bj = load double, double* %bjp, align 8
  %bj.next = fadd nnan nsz arcp afn reassoc double %bj, %Aij
  store double %bj.next, double* %bjp, align 8
  %j.next = add nuw nsw i64 %j, 1
  %L2.cond = icmp ne i64 %j.next, 32
  br i1 %L2.cond, label %L2, label %L1.latch

L1.latch:
  %i.next = add nuw nsw i64 %i, 1
  %L1.cond = icmp ne i64 %i.next, 64
  br i1 %L1.cond, label %L1, label %exit

exit:
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }
