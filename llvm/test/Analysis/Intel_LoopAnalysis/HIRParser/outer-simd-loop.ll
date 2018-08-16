; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Check that we are able to parse simd on outer loops correctly.
; We should be able to handle cases where simd intrinsics are outside the ztt.

;CHECK: %1 = bitcast.i64*.i8*(&((%.omp.ub)[0]));
;CHECK: @llvm.lifetime.start.p0i8(8,  &((i8*)(%.omp.ub)[0]));
;CHECK: (%.omp.ub)[0] = %N + -1;
;CHECK: @llvm.intel.directive(!"DIR.OMP.SIMD");
;CHECK: @llvm.intel.directive.qual.opndlist(!"QUAL.OMP.NORMALIZED.IV",  &((%.omp.iv)[0]));
;CHECK: @llvm.intel.directive.qual.opndlist(!"QUAL.OMP.NORMALIZED.UB",  &((%.omp.ub)[0]));
;CHECK: @llvm.intel.directive(!"DIR.QUAL.LIST.END");
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

;CHECK: @llvm.intel.directive(!"DIR.OMP.END.SIMD");
;CHECK: @llvm.intel.directive(!"DIR.QUAL.LIST.END");

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo(i64 %N, i64* nocapture readnone %lb, i64* nocapture readonly %ub) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %0 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  %cmp = icmp sgt i64 %N, 0
  br i1 %cmp, label %omp.precond.then, label %entry.omp.precond.end_crit_edge

entry.omp.precond.end_crit_edge:                  ; preds = %entry
  %.pre = bitcast i64* %.omp.ub to i8*
  br label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i64 %N, -1
  %1 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1) #2
  store i64 %sub2, i64* %.omp.ub, align 8, !tbaa !2
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.NORMALIZED.IV", i64* nonnull %.omp.iv)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.NORMALIZED.UB", i64* nonnull %.omp.ub)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  store i64 0, i64* %.omp.iv, align 8, !tbaa !2
  %2 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp423 = icmp slt i64 %2, 0
  br i1 %cmp423, label %omp.loop.exit, label %for.cond.preheader.preheader

for.cond.preheader.preheader:                     ; preds = %omp.precond.then
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %for.cond.preheader.preheader, %omp.inner.for.inc
  %storemerge24 = phi i64 [ %add10, %omp.inner.for.inc ], [ 0, %for.cond.preheader.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %ub, i64 %storemerge24
  %3 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %cmp621 = icmp sgt i64 %3, 0
  br i1 %cmp621, label %for.body.preheader, label %omp.inner.for.inc

for.body.preheader:                               ; preds = %for.cond.preheader
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.022 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %add7 = add nuw nsw i64 %j.022, %storemerge24
  %conv = sitofp i64 %add7 to float
  %arrayidx9 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %j.022, i64 %storemerge24
  store float %conv, float* %arrayidx9, align 4, !tbaa !6
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
  store i64 %add10.lcssa, i64* %.omp.iv, align 8, !tbaa !2
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge, %omp.precond.then
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %entry.omp.precond.end_crit_edge, %omp.loop.exit
  %.pre-phi = phi i8* [ %.pre, %entry.omp.precond.end_crit_edge ], [ %1, %omp.loop.exit ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %.pre-phi) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 1bfc9df47817f0ad6a4bbfc288ea20461fe3701d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm eb2cff02098f6e2716b595c3fa2c05b073fc7032)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !9, i64 0}
!7 = !{!"array@_ZTSA100_A100_f", !8, i64 0}
!8 = !{!"array@_ZTSA100_f", !9, i64 0}
!9 = !{!"float", !4, i64 0}
