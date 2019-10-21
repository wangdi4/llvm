; Test to check that VPlan's LLVM-IR based vector code generator can explicitly handle trunc without underlying IR Value.
; This test is based on Transforms/Intel_VPO/Vecopt/vplan_vectorize_zext_without_ir.ll

; Explictly disable VPValue-CG, this test is specifically for IR-based CG.
; RUN: opt -S < %s -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen=false | FileCheck %s

; CHECK: [[trunc_1:%.*]] = trunc <2 x i64> {{.*}} to <2 x i32>
; CHECK-NEXT: [[GEP_1:%.*]] = getelementptr inbounds i32, <2 x i32*> [[ARR_1:%.*]], <2 x i32> [[trunc_1]]
; CHECK: [[trunc_2:%.*]] = trunc <2 x i64> {{.*}} to <2 x i32>
; CHECK-NEXT: [[GEP_2:%.*]] = getelementptr inbounds i32, <2 x i32*> [[ARR_1]], <2 x i32> [[trunc_2]]

target datalayout = "e-m:e-i32:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @foo_non_lcssa(i64 %N, i32 *%a, i64 %mask_out_inner_loop) local_unnamed_addr {
entry:
  %cmp18 = icmp sgt i64 %N, 0
  br i1 %cmp18, label %for.cond1.preheader.preheader, label %for.end7

for.cond1.preheader.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:
  %outer.iv = phi i64 [ %outer.iv.next, %for.inc5 ], [ 0, %for.cond1.preheader.preheader ]
  %skip_loop = icmp eq i64 %outer.iv, %mask_out_inner_loop
  br i1 %skip_loop, label %for.inc5, label %top_test

top_test:
  %cmp216 = icmp eq i64 %outer.iv, 0
  br i1 %cmp216, label %for.inc5, label %for.body3.preheader

for.body3.preheader:
  br label %for.body3

for.body3:
  %inner.iv = phi i64 [ %inner.iv.next, %no_early_exit ], [ 0, %for.body3.preheader ]
  %ext = trunc i64 %inner.iv to i32
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %ext
  %ld = load i32, i32* %arrayidx
  %some_cmp = icmp eq i32 %ld, 42
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  br i1 %some_cmp, label %for.inc5.loopexit, label %no_early_exit

no_early_exit:
  %exitcond = icmp eq i64 %inner.iv.next, %outer.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:
  %phi_use = phi i64 [ %inner.iv, %for.body3 ], [ 100, %no_early_exit ]
  %ext1 = trunc i64 %phi_use to i32
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i32 %ext1
  store i32 125, i32* %arrayidx1, align 8
  br label %for.inc5

for.inc5:
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %outer_exit_cond = icmp eq i64 %outer.iv.next, %N
  br i1 %outer_exit_cond, label %for.end7.loopexit, label %for.cond1.preheader

for.end7.loopexit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end7

for.end7:
  ret void
}
