; Regression test from CMPLRLLVM-8804. Test that the block predicate is placed at
; correct location before the store.
; REQUIRES: asserts
; RUN: opt -S -VPlanDriver %s -debug 2>&1 | FileCheck %s
; CHECK: VPlan after predication
; CHECK-COUNT-6: BB{{[0-9]+}} {{.*}} :
; CHECK-NEXT: block-predicate
; CHECK-NEXT: call {{.*}} @foo
; CHECK-NEXT: [[LRES:%vp.*]] = load i32* [[LDADDR:%vp.*]]
; CHECK-NEXT: [[ADDRES:%vp.*]] = add i32 [[LRES]] i32 1
; CHECK-NEXT:   store i32 [[ADDRES]] i32* [[LDADDR]]
; ModuleID = 't6.c'
source_filename = "t6.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @test(i32* nocapture %p, i32 %b, i32 %e, i32 %n) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %j.priv = alloca i32, align 4
  %j.priv.priv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(i32* %j.priv) ]
  br label %blk1

blk1:
  %1 = bitcast i32* %j.priv.priv to i8*
  %2 = zext i32 %e to i64
  %3 = zext i32 %b to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1.split
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.body.continue ], [ 0, %blk1 ]
  %cmp1 = icmp uge i64 %indvars.iv, %3
  %cmp2 = icmp ult i64 %indvars.iv, %2
  %or.cond = and i1 %cmp1, %cmp2
  br i1 %or.cond, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 0, i32* %j.priv.priv, align 4
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  br label %omp.body.continue

for.body:                                         ; preds = %if.then, %for.body
  call void (...) @foo() #2
  %4 = load i32, i32* %arrayidx, align 4
  %inc = add i32 %4, 1
  store i32 %inc, i32* %arrayidx, align 4
  %5 = load i32, i32* %j.priv.priv, align 4
  %inc4 = add i32 %5, 1
  store i32 %inc4, i32* %j.priv.priv, align 4
  %cmp3 = icmp ult i32 %inc4, 10
  br i1 %cmp3, label %for.body, label %for.cond.cleanup

omp.body.continue:                                ; preds = %omp.inner.for.body, %for.cond.cleanup
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %retblk

retblk:
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local void @foo(...) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1
