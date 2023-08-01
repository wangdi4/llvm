;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -aa-pipeline=basic-aa -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
; LIT test to demonstrate dependence checks preventing VLS optimization from
; kicking in. Incoming HIR looks like the following:
;
; <23>     + DO i1 = 0, 1023, 1   <DO_LOOP>
; <6>      |   (%lp2)[i1] = (%lp1)[2 * i1];
; <10>     |   %mul4 = (%lp1)[3 * i1]  *  3.000000e+00;
; <14>     |   %add7 = %mul4  +  (%lp1)[2 * i1 + 1];
; <16>     |   (%lp3)[i1] = %add7;
; <23>     + END LOOP
;
; VLS analysis checks in VPlan HIR path are done using HIR. When trying
; to see if we can move <14> to <6>, we run into the flow dependence
; between <10> and <14> which prevents this move. However, VLS transform
; only moves the load and not the entire HLInst and in this case this HLInst
; level check unnecessarily prevents VLS from kicking in. This is one of the
; issues which prevents ifx/ifort performance parity for the customer reported
; benchmark in CMPLRLLVM-39502. TODO: VLS analysis checks need to be moved
; to VPlan level in the HIR path after we can disambiguate accesses using
; alias information.
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:      |   %.vls.load = (<8 x float>*)(%lp1)[2 * i1];
; CHECK-NEXT:      |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:      |   %vls.extract1 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:      |   (<4 x float>*)(%lp2)[i1] = %vls.extract;
; CHECK-NEXT:      |   %.vec = (<4 x float>*)(%lp1)[3 * i1 + 3 * <i64 0, i64 1, i64 2, i64 3>];
; CHECK-NEXT:      |   %.vec2 = %.vec  *  3.000000e+00;
; CHECK-NEXT:      |   %.vec3 = %.vec2  +  %vls.extract1;
; CHECK-NEXT:      |   (<4 x float>*)(%lp3)[i1] = %.vec3;
; CHECK-NEXT:      + END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sum(ptr noalias %lp1, ptr noalias %lp2, ptr noalias %lp3) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.019 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.019, 1
  %arrayidx = getelementptr inbounds float, ptr %lp1, i64 %mul
  %0 = load float, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds float, ptr %lp2, i64 %l1.019
  store float %0, ptr %arrayidx1, align 4
  %mul2 = mul nuw nsw i64 %l1.019, 3
  %arrayidx3 = getelementptr inbounds float, ptr %lp1, i64 %mul2
  %1 = load float, ptr %arrayidx3, align 4
  %mul4 = fmul fast float %1, 3.000000e+00
  %add = add nuw nsw i64 %mul, 1
  %arrayidx6 = getelementptr inbounds float, ptr %lp1, i64 %add
  %2 = load float, ptr %arrayidx6, align 4
  %add7 = fadd fast float %mul4, %2
  %arrayidx8 = getelementptr inbounds float, ptr %lp3, i64 %l1.019
  store float %add7, ptr %arrayidx8, align 4
  %inc = add nuw nsw i64 %l1.019, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
