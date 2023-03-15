;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -hir-details < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate issue with generating code for select inside an inner loop
; when vectorizing an outer loop. When generating the select mask, in order for HIR
; idiom recognition to work, we need to use operands of the compare instruction
; corresponding to the select mask when possible. While doing this we need to
; mark the compare operands as livein to the inner loop.
;
; Example:
;   Outer loop:
;      ...
;      cmp = op1 pred op2
;
;      Inner loop:
;         ...
;         select cmp, val1, val2
;
; For idiom recognition to work, we generate HIR for the select that looks like:
;
;         select (op1 pred op2) ? val1 : val2
;
; We need to mark op1 and op2 as live-in for the inner loop. Otherwise, we hit
; HIR verification errors.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;
; CHECK:  DO i64 i1 = 0, 7, 2   <DO_LOOP> <simd-vectorized> <novectorize>
;
; CHECK:    %.vec3 = %.vec > %farg; <fast>
; CHECK:    <RVAL-REG> NON-LINEAR <2 x float> %.vec {sb:[[VECSB:[0-9]+]]}
; CHECK:    <RVAL-REG> LINEAR <2 x float> %farg {sb:[[ARGSB:[0-9]+]]}
;
; CHECK:         LiveIn symbases: [[ARGSB]]{{.*}}[[VECSB]]
; CHECK:         UNKNOWN LOOP i2 <novectorize>
;
; CHECK:           %.vec6 = (%.vec > %farg) ? %.vec5 : 0;
; CHECK:           <RVAL-REG> NON-LINEAR <2 x float> %.vec {sb:[[VECSB]]}
; CHECK:           <RVAL-REG> LINEAR <2 x float> %farg {sb:[[ARGSB]]}
;
; CHECK:         END LOOP
;
; CHECK:  END LOOP
define void @baz(ptr noalias %lp1, ptr noalias %lp2, ptr noalias %fp, float %farg) {
entry:
  br label %outer.ph

outer.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %outer.for.body

outer.for.body:
  %outer.iv = phi i64 [ 0, %outer.ph ], [ %outeriv.inc, %outer.for.inc ]
  %arrayidx = getelementptr inbounds float, ptr %fp, i64 %outer.iv
  %1 = load float, ptr %arrayidx, align 4
  %cmp1 = fcmp fast ogt float %1, %farg
  %outeriv.inc = add nuw nsw i64 %outer.iv, 1
  br i1 %cmp1, label %for.cond.preheader, label %outer.for.inc

for.cond.preheader:
  %arrayidx2 = getelementptr inbounds float, ptr %fp, i64 %outeriv.inc
  %fval = load float, ptr %arrayidx2, align 4
  %cmp4 = fcmp fast ogt float %fval, 2.000000e+01
  %arrayidx6 = getelementptr inbounds i64, ptr %lp1, i64 %outer.iv
  br label %for.body

for.body:
  %l2.024 = phi i64 [ 0, %for.cond.preheader ], [ %inc, %for.inc ]
  br i1 %cmp4, label %if.then5, label %for.inc

if.then5:
  %2 = load i64, ptr %arrayidx6, align 8
  %cmp7 = icmp eq i64 %2, 10
  %conv8 = zext i1 %cmp7 to i64
  %arrayidx9 = getelementptr inbounds i64, ptr %lp2, i64 %l2.024
  store i64 %conv8, ptr %arrayidx9, align 8
  br label %for.inc

for.inc:
  %inc = add nuw nsw i64 %l2.024, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %outer.for.inc.loopexit, label %for.body

outer.for.inc.loopexit:
  br label %outer.for.inc

outer.for.inc:
  %exitcond25.not = icmp eq i64 %outeriv.inc, 8
  br i1 %exitcond25.not, label %exit, label %outer.for.body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
