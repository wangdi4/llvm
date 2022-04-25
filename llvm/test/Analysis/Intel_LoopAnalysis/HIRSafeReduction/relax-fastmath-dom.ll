; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-safe-reduction-analysis -S 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -S 2>&1 < %s | FileCheck %s

; [Note] Safe reduction is on dom path (without unsafe algebra)

; Function foo's HIR dump: %sum.07 is a safe reduction without Unsafe Algebra
;
;  <0>          BEGIN REGION { }
;  <11>               + DO i1 = 0, 1023, 1   <DO_LOOP>
;  <3>                |   %0 = (@arr)[0][i1];
;  <4>                |   %sum.07 = %0  +  %sum.07;
;  <11>               + END LOOP
;  <0>          END REGION


; CHECK:        + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK:        |   <Safe Reduction> Red Op: fadd <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:        |   %sum.07 = %0  +  %sum.07; <Safe Reduction>
; CHECK:        + END LOOP


@arr = common local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define float @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.07 = phi float [ 0.0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %add = fadd reassoc float %0, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  ret float %add.lcssa
}
