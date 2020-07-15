; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -debug -S < %s 2>&1  | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -debug -S < %s 2>&1  | FileCheck %s
;
; Test src:
; void test_01(float *a, float *b, int size) {
; #pragma omp simd nontemporal(a, b)
;   for (int i = 0; i < size; ++i) {
;     float tmp = b[i];
;     a[i] = tmp * tmp + tmp + 1.0;
;   }
; }
;
; Check that we can parse the NONTEMPORAL clause.
;
; CHECK: NONTEMPORAL clause (size=2): (float** %a.addr) (float** %b.addr)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z7test_01PfS_i(float* %a, float* %b, i32 %size) local_unnamed_addr #0 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %a, float** %a.addr, align 8, !tbaa !2
  store float* %b, float** %b.addr, align 8, !tbaa !2
  %cmp = icmp sgt i32 %size, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre = bitcast i32* %.omp.ub to i8*
  %.pre9 = bitcast i32* %.omp.iv to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i32 %size, -1
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store i32 %sub2, i32* %.omp.ub, align 4, !tbaa !6
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL"(float** %a.addr, float** %b.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4, !tbaa !6
  %3 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp37 = icmp slt i32 %3, 0
  br i1 %cmp37, label %omp.loop.exit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  %4 = bitcast i32* %i to i8*
  %5 = load float*, float** %b.addr, align 8, !tbaa !2
  %6 = load float*, float** %a.addr, align 8, !tbaa !2
  %7 = add nuw i32 %3, 1
  %wide.trip.count = zext i32 %7 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #2
  %ptridx = getelementptr inbounds float, float* %5, i64 %indvars.iv
  %8 = load float, float* %ptridx, align 4, !tbaa !8
  %mul5 = fmul float %8, %8
  %add6 = fadd float %8, %mul5
  %conv8 = fadd float %add6, 1.000000e+00
  %ptridx10 = getelementptr inbounds float, float* %6, i64 %indvars.iv
  store float %conv8, float* %ptridx10, align 4, !tbaa !8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit_crit_edge:       ; preds = %omp.inner.for.body
  %9 = add i32 %3, 1
  store i32 %9, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %DIR.OMP.END.SIMD.3
  %.pre-phi10 = phi i8* [ %.pre9, %entry.omp.precond.end_crit_edge ], [ %0, %DIR.OMP.END.SIMD.3 ]
  %.pre-phi = phi i8* [ %.pre, %entry.omp.precond.end_crit_edge ], [ %1, %DIR.OMP.END.SIMD.3 ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %.pre-phi10) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !4, i64 0}
