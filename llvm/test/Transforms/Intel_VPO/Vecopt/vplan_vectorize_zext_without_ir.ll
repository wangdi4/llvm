; Test to check that VPlan's LLVM-IR based vector code generator can explicitly handle ZExt without underlying IR Value.
; This test is based on Transforms/Intel_VPO/Vecopt/vplan_loopcfu_liveout_nonlcssa.ll

; Explictly disable VPValue-CG, this test is specifically for IR-based CG.
; RUN: opt -S < %s -VPlanDriver -vplan-force-vf=2 -enable-vp-value-codegen=false | FileCheck %s

; CHECK: [[ZEXT_1:%.*]] = zext <2 x i32> {{.*}} to <2 x i64>
; CHECK-NEXT: [[GEP_1:%.*]] = getelementptr inbounds i64, <2 x i64*> [[ARR_1:%.*]], <2 x i64> [[ZEXT_1]]
; CHECK: [[ZEXT_2:%.*]] = zext <2 x i32> {{.*}} to <2 x i64>
; CHECK-NEXT: [[GEP_2:%.*]] = getelementptr inbounds i64, <2 x i64*> [[ARR_1]], <2 x i64> [[ZEXT_2]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @foo_non_lcssa(i32 %N, i64 *%a, i32 %mask_out_inner_loop) local_unnamed_addr {
entry:
  %cmp18 = icmp sgt i32 %N, 0
  br i1 %cmp18, label %for.cond1.preheader.preheader, label %for.end7

for.cond1.preheader.preheader:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:
  %outer.iv = phi i32 [ %outer.iv.next, %for.inc5 ], [ 0, %for.cond1.preheader.preheader ]
  %skip_loop = icmp eq i32 %outer.iv, %mask_out_inner_loop
  br i1 %skip_loop, label %for.inc5, label %top_test

top_test:
  %cmp216 = icmp eq i32 %outer.iv, 0
  br i1 %cmp216, label %for.inc5, label %for.body3.preheader

for.body3.preheader:
  br label %for.body3

for.body3:
  %inner.iv = phi i32 [ %inner.iv.next, %no_early_exit ], [ 0, %for.body3.preheader ]
  %ext = zext i32 %inner.iv to i64
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %ext
  %ld = load i64, i64* %arrayidx
  %some_cmp = icmp eq i64 %ld, 42
  %inner.iv.next = add nuw nsw i32 %inner.iv, 1
  br i1 %some_cmp, label %for.inc5.loopexit, label %no_early_exit

no_early_exit:
  %exitcond = icmp eq i32 %inner.iv.next, %outer.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:
  %phi_use = phi i32 [ %inner.iv, %for.body3 ], [ 100, %no_early_exit ]
  %phi_update_use = phi i32 [ %inner.iv.next, %for.body3 ], [ 100, %no_early_exit ]
  %no_phi_inst_use = phi i1 [%some_cmp, %for.body3 ], [ 100, %no_early_exit ]
  %use_a = add i32 %phi_use, 1
  %use_b = add i32 %phi_update_use, 1
  %ext1 = zext i32 %phi_use to i64
  %arrayidx1 = getelementptr inbounds i64, i64* %a, i64 %ext1
  store i64 125, i64* %arrayidx1, align 8
  %use_c = xor i1 %no_phi_inst_use, -1
  %problem_use = add i32 %use_a, %use_b
  %problem_use2 = and i1 %use_c, 1
  br label %for.inc5

for.inc5:
  %outer.iv.next = add nuw nsw i32 %outer.iv, 1
  %outer_exit_cond = icmp eq i32 %outer.iv.next, %N
  br i1 %outer_exit_cond, label %for.end7.loopexit, label %for.cond1.preheader

for.end7.loopexit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.end7

for.end7:
  ret void
}
