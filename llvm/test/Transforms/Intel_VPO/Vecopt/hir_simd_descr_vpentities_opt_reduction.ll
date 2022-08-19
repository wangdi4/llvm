; Test to check that a VPlan loop entity is not created for a reduction that is optimized out by pre-vectorizer optimizations.

; #define C1 1
; #define C2 2
; 
; int a[1024];
; int b[1024];
; int c[1024];
; 
; int foo() {
;     int s = 0;
; #pragma omp simd reduction(+:s)
;     for (int i = 0; i < 1024; i++) {
;         s += C1 * C2;
;         a[i] = b[i] + c[i] * C2;
;     }
;     return s;
; }

; Input HIR
; <0>     BEGIN REGION { }
; <2>           %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%s)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ] 
; <3>           %s.promoted = (%s)[0];
; <28>          
; <28>          + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
; <8>           |   %2 = (@b)[0][i1];
; <10>          |   %3 = (@c)[0][i1];
; <14>          |   (@a)[0][i1] = %2 + 2 * %3;
; <28>          + END LOOP
; <28>               
; <23>         (%s)[0] = %s.promoted + 2048;
; <24>         @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ] 
; <25>         %5 = (%s)[0];
; <26>         @llvm.lifetime.end.p0i8(4,  &((i8*)(%s)[0]));
; <27>         ret %5;
; <0>     END REGION

; In above HIR the reduction is optimized out and there are no uses within the loop. Ensure that a VPlan entity is not created for it.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-vplan-codegen -vplan-entities-dump -debug -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-vplan-codegen -vplan-entities-dump -debug -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -disable-vplan-codegen -vplan-entities-dump -debug -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -disable-vplan-codegen -vplan-entities-dump -debug -disable-output < %s 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; REQUIRES: asserts
; CHECK-NOT: Reduction list

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3foov() local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %s = alloca i32, align 4
  %0 = bitcast i32* %s to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  store i32 0, i32* %s, align 4, !tbaa !2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(i32* %s, i32 0, i32 1) ]
  %s.promoted = load i32, i32* %s, align 4, !tbaa !2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %arrayidx3 = getelementptr inbounds [1024 x i32], [1024 x i32]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !6
  %mul4 = shl i32 %3, 1
  %add5 = add nsw i32 %mul4, %2
  %arrayidx7 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !6
  store i32 %add5, i32* %arrayidx7, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %4 = add i32 %s.promoted, 2048
  store i32 %4, i32* %s, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  %5 = load i32, i32* %s, align 4, !tbaa !2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 %5
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1024_i", !3, i64 0}
