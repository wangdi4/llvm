; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check that we are able to parse simd on outer loops correctly.
; We should be able to handle cases where simd intrinsics are outside the ztt.

;CHECK: %t4 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]
;CHECK: (%.omp.iv)[0] = 0;
;CHECK: %2 = (%.omp.ub)[0];

;CHECK: + DO i1 = 0, %2, 1   <DO_LOOP>
;CHECK: |   %3 = (%ub)[i1];
;CHECK: |
;CHECK: |   + DO i2 = 0, %3 + -1, 1   <DO_LOOP>
;CHECK: |   |   %conv = sitofp.i64.float(i1 + i2);
;CHECK: |   |   (@A)[0][i2][i1] = %conv;
;CHECK: |   + END LOOP
;CHECK: |
;CHECK: |   %add10 = i1  +  1;
;CHECK: + END LOOP
;CHECK: (%.omp.iv)[0] = %add10;

;CHECK: @llvm.directive.region.exit(%t4); [ DIR.OMP.END.SIMD() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo(i64 %N, ptr nocapture readnone %lb, ptr nocapture readonly %ub) local_unnamed_addr {
entry:
  %.omp.iv = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %0 = bitcast ptr %.omp.iv to ptr
  %cmp = icmp sgt i64 %N, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre = bitcast ptr %.omp.ub to ptr
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i64 %N, -1
  %1 = bitcast ptr %.omp.ub to ptr
  store i64 %sub2, ptr %.omp.ub, align 8
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  store i64 0, ptr %.omp.iv, align 8
  %2 = load i64, ptr %.omp.ub, align 8
  %cmp423 = icmp slt i64 %2, 0
  br i1 %cmp423, label %omp.loop.exit, label %for.cond.preheader.preheader

for.cond.preheader.preheader:                     ; preds = %omp.precond.then
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %for.cond.preheader.preheader, %omp.inner.for.inc
  %storemerge24 = phi i64 [ %add10, %omp.inner.for.inc ], [ 0, %for.cond.preheader.preheader ]
  %arrayidx = getelementptr inbounds i64, ptr %ub, i64 %storemerge24
  %3 = load i64, ptr %arrayidx, align 8
  %cmp621 = icmp sgt i64 %3, 0
  br i1 %cmp621, label %for.body.preheader, label %omp.inner.for.inc

for.body.preheader:                               ; preds = %for.cond.preheader
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.022 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %add7 = add nuw nsw i64 %j.022, %storemerge24
  %conv = sitofp i64 %add7 to float
  %arrayidx9 = getelementptr inbounds [100 x [100 x float]], ptr @A, i64 0, i64 %j.022, i64 %storemerge24
  store float %conv, ptr %arrayidx9, align 4
  %inc = add nuw nsw i64 %j.022, 1
  %cmp6 = icmp slt i64 %inc, %3
  br i1 %cmp6, label %for.body, label %omp.inner.for.inc.loopexit

omp.inner.for.inc.loopexit:                       ; preds = %for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.inc.loopexit, %for.cond.preheader
  %add10 = add nuw nsw i64 %storemerge24, 1
  %cmp4 = icmp slt i64 %storemerge24, %2
  br i1 %cmp4, label %for.cond.preheader, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.inc
  %add10.lcssa = phi i64 [ %add10, %omp.inner.for.inc ]
  store i64 %add10.lcssa, ptr %.omp.iv, align 8
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %omp.precond.then
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %omp.loop.exit
  %.pre-phi = phi ptr [ %.pre, %entry.omp.precond.end_crit_edge ], [ %1, %omp.loop.exit ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


