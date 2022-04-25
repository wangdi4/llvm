; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 -hir-create-function-level-region | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-create-function-level-region 2>&1 | FileCheck %s

; Unknown loop header labels can cause problems for the structured control flow
; check in domination queries, so this test verifies that instructions preceding
; an unknown loop dominate ones in its body for a problematic unknown loop
; containing other labels. This is tested by checking for edges with a (=) DV
; in the DDG, which are constructed for temp domination.

; BEGIN REGION
;       %x = 0;
;
;       + DO i1 = 0, 64, 1   <DO_LOOP>
;       |   %A = (%As)[i1];
;       |
;       |   + UNKNOWN LOOP i2
;       |   |   <i2 = 0>
;       |   |   L1:
;       |   |   %x.out3 = %x;
;       |   |   %Ai = (%A)[i2];
;       |   |   if (%Ai < 5.000000e-01)
;       |   |   {
;       |   |      %x = %x  +  1;
;       |   |      %x.AB = %x;
;       |   |      if (%Ai < 2.500000e-01)
;       |   |      {
;       |   |         %x = %x  +  3;
;       |   |         goto L1.latch;
;       |   |      }
;       |   |   }
;       |   |   else
;       |   |   {
;       |   |      %x.AB = %x.out3 + 2;
;       |   |   }
;       |   |   %x = %x.AB + 4;
;       |   |   L1.latch:
;       |   |   if (%Ai != 0.000000e+00)
;       |   |   {
;       |   |      <i2 = i2 + 1>
;       |   |      goto L1;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
;
;       ret %x;
; END REGION

; CHECK: %A --> %A FLOW (=) (0)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @temp-domination-unknown-header(double** %As) {
entry:
  br label %L0

L0:
  %n = phi i64 [ 0, %entry ], [ %n.next, %L0.latch ]
  %x = phi i32 [ 0, %entry ], [ %x.lcssa, %L0.latch ]
  %Asn = getelementptr inbounds double*, double** %As, i64 %n
  %A = load double*, double** %Asn
  br label %L1

L1:
  %i = phi i64 [ 0, %L0 ], [ %i.next, %L1.latch ]
  %x.L1 = phi i32 [ %x, %L0 ], [ %x.CD, %L1.latch ]
  %Aip = getelementptr inbounds double, double* %A, i64 %i
  %Ai = load double, double* %Aip
  %AB.cond = fcmp olt double %Ai, 0.5
  br i1 %AB.cond, label %L1.A, label %L1.B

L1.A:
  %x.A = add i32 %x.L1, 1
  %CD.cond = fcmp olt double %Ai, 0.25
  br i1 %CD.cond, label %L1.C, label %L1.D

L1.B:
  %x.B = add i32 %x.L1, 2
  br label %L1.D

L1.C:
  %x.C = add i32 %x.A, 3
  br label %L1.latch

L1.D:
  %x.AB = phi i32 [ %x.A, %L1.A ], [ %x.B, %L1.B ]
  %x.D = add i32 %x.AB, 4
  br label %L1.latch

L1.latch:
  %x.CD = phi i32 [ %x.C, %L1.C ], [ %x.D, %L1.D ]
  %i.next = add nuw nsw i64 %i, 1
  %L1.cond = fcmp one double %Ai, 0.0
  br i1 %L1.cond, label %L1, label %L0.latch

L0.latch:
  %x.lcssa = phi i32 [ %x.CD, %L1.latch ]
  %n.next = add nuw nsw i64 %n, 1
  %L0.cond = icmp ne i64 %n, 64
  br i1 %L0.cond, label %L0, label %exit

exit:
  %x.lcssa.lcssa = phi i32 [ %x.lcssa, %L0.latch ]
  ret i32 %x.lcssa.lcssa
}
