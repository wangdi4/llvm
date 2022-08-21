; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo(float *A, float *B, char *C, char *D, int N) {
; #pragma omp simd nontemporal(A, B, C, D)
;   for (int I = 0; I < N; ++I) {
;     *((int*) &B[I]) = *((int *) &A[I]);
;     ((int *) C)[I] = ((int *) D)[I];
;   }
; }

; Check that we can parse the NONTEMPORAL clause and translate it into
; !nontemporal metadata even in presence of bitcasts.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr noundef %A, ptr noundef %B, ptr noundef %C, ptr noundef %D, i32 noundef %N) #0 {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %C.addr = alloca ptr, align 8
  %D.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8, !tbaa !4
  store ptr %B, ptr %B.addr, align 8, !tbaa !4
  store ptr %C, ptr %C.addr, align 8, !tbaa !8
  store ptr %D, ptr %D.addr, align 8, !tbaa !8
  store i32 %N, ptr %N.addr, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0) #2
  %0 = load i32, ptr %N.addr, align 4, !tbaa !10
  store i32 %0, ptr %.capture_expr.0, align 4, !tbaa !10
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.1) #2
  %1 = load i32, ptr %.capture_expr.0, align 4, !tbaa !10
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4, !tbaa !10
  %2 = load i32, ptr %.capture_expr.0, align 4, !tbaa !10
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  %3 = load i32, ptr %.capture_expr.1, align 4, !tbaa !10
  store i32 %3, ptr %.omp.ub, align 4, !tbaa !10
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %A.addr),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %B.addr),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %C.addr),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %D.addr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]
  store i32 0, ptr %.omp.iv, align 4, !tbaa !10
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !10
  %6 = load i32, ptr %.omp.ub, align 4, !tbaa !10
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %I) #2
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !10
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4, !tbaa !10
  %8 = load ptr, ptr %A.addr, align 8, !tbaa !4
  %9 = load i32, ptr %I, align 4, !tbaa !10
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds float, ptr %8, i64 %idxprom
  %bc1 = bitcast ptr %arrayidx to ptr
  ; CHECK: [[LD1:%.*]] = load i32, ptr %{{.*}}, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  %10 = load i32, ptr %bc1, align 4, !tbaa !10
  %11 = load ptr, ptr %B.addr, align 8, !tbaa !4
  %12 = load i32, ptr %I, align 4, !tbaa !10
  %idxprom5 = sext i32 %12 to i64
  %arrayidx6 = getelementptr inbounds float, ptr %11, i64 %idxprom5
  %bc2 = bitcast ptr %arrayidx6 to ptr
  ; CHECK: store i32 [[LD1]], ptr %{{.*}}, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  store i32 %10, ptr %bc2, align 4, !tbaa !10
  %13 = load ptr, ptr %D.addr, align 8, !tbaa !8
  %14 = load i32, ptr %I, align 4, !tbaa !10
  %idxprom7 = sext i32 %14 to i64
  %arrayidx8 = getelementptr inbounds i32, ptr %13, i64 %idxprom7
  ; CHECK: [[LD2:%.*]] = load i32, ptr %arrayidx8, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  %15 = load i32, ptr %arrayidx8, align 4, !tbaa !10
  %16 = load ptr, ptr %C.addr, align 8, !tbaa !8
  %17 = load i32, ptr %I, align 4, !tbaa !10
  %idxprom9 = sext i32 %17 to i64
  %arrayidx10 = getelementptr inbounds i32, ptr %16, i64 %idxprom9
  ; CHECK: store i32 [[LD2]], ptr %arrayidx10, align 4, !tbaa !{{[0-9]*}}, !nontemporal
  store i32 %15, ptr %arrayidx10, align 4, !tbaa !10
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %I) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %18 = load i32, ptr %.omp.iv, align 4, !tbaa !10
  %add11 = add nsw i32 %18, 1
  store i32 %add11, ptr %.omp.iv, align 4, !tbaa !10
  br label %omp.inner.for.cond, !llvm.loop !12

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.1) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSPf", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPc", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !6, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.vectorize.enable", i1 true}
