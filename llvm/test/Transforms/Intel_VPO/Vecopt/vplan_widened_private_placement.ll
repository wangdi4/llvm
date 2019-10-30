; This test makes sure that we do not crash in the vectorizer when
; we declare a global array as loop-private.
; We obtain the Function-entry block where we want to put the widened
; private via the Original Loop and not depend on the assumption
; that the original private variable is an 'AllocaInst'.

; Run the following command and intercept the function and print module
; before VPlanDriver is invoked on a function.
;
; icx -Xclang -fintel-openmp-region -c -fopenmp -mllvm --vplan-driver
; -mllvm -vplan-force-vf=2 -mllvm --loopopt=0 <input>.cpp
;
; Source code
; const int N=1024;
; int arr[N];
; int i = 0, j = 0;
; int getElement(int RetIdx) {
; #pragma omp simd private(arr)
;   for (i = 0; i < N; ++i)
;     for (j = 0; j < N; ++j) {
;       arr[j] = i * j;
;     }
;   return arr[RetIdx];
; }


; RUN: opt -VPlanDriver -vplan-force-vf=2 -S  -enable-vp-value-codegen=false %s | FileCheck %s --check-prefixes=CHECK,CHECK-LLVM
; RUN: opt -VPlanDriver -vplan-force-vf=2 -S  -enable-vp-value-codegen %s | FileCheck %s --check-prefixes=CHECK,CHECK-VPVALUE

; CHECK: entry:
; CHECK-LLVM-NEXT: {{.*}} = alloca [2 x [1024 x i32]], align 16
; CHECK-VPVALUE: [[PRIV1:%.*]] = alloca [2 x [1024 x i32]], align 4
; CHECK-VPVALUE-NEXT: {{.*}} = bitcast [2 x [1024 x i32]]* [[PRIV1]] to [1024 x i32]*


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local global [1024 x i32] zeroinitializer, align 16
@i = dso_local global i32 0, align 4
@j = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z10getElementi(i32 %RetIdx) local_unnamed_addr {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  store i32 1023, i32* %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* @arr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LASTPRIVATE"(i32* @i) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp9 = icmp slt i32 %3, 0
  br i1 %cmp9, label %omp.loop.exit, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %DIR.OMP.SIMD.2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.preheader, %omp.inner.for.inc
  %storemerge10 = phi i32 [ %add3, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.preheader ]
  br label %for.body

for.body:                                         ; preds = %for.body, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %omp.inner.for.body ], [ %indvars.iv.next, %for.body ]
  %4 = trunc i64 %indvars.iv to i32
  %mul2 = mul nsw i32 %storemerge10, %4
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  store i32 %mul2, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.inc, label %for.body

omp.inner.for.inc:                                ; preds = %for.body
  %add3 = add nuw nsw i32 %storemerge10, 1
  %cmp = icmp slt i32 %storemerge10, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.cond.omp.loop.exit_crit_edge

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.inc
  %add3.lcssa = phi i32 [ %add3, %omp.inner.for.inc ]
  %storemerge10.lcssa = phi i32 [ %storemerge10, %omp.inner.for.inc ]
  store i32 1024, i32* @j, align 4
  store i32 %storemerge10.lcssa, i32* @i, align 4
  store i32 %add3.lcssa, i32* %.omp.iv, align 4
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  %idxprom4 = sext i32 %RetIdx to i64
  %arrayidx5 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %idxprom4
  %5 = load i32, i32* %arrayidx5, align 4
  ret i32 %5
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
