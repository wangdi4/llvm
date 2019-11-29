; Test to check that DA marks StructType pointer GEPs with appropriate shape.

; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -debug-only=vplan-divergence-analysis -vplan-force-vf=4 -S 2>&1 | FileCheck %s

; Check DA results
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[IV:%vp.*]] = phi  [ i64 [[IV_ADD:%vp.*]], BB3 ],  [ i64 0, BB2 ]
; Check that non-uniform GEPs on StructType are divergent
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 8] %struct.S* [[BASE:%vp.*]] = getelementptr inbounds %struct.S* %SArr i64 [[IV]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 8] i32* [[ELEMENT:%vp.*]] = getelementptr inbounds %struct.S* [[BASE]] i32 0 i32 0
; CHECK-NEXT: Divergent: [Shape: Random] i32 [[TRUNC:%vp.*]] = trunc i64 [[IV]] to i32
; CHECK-NEXT: Divergent: [Shape: Random] store i32 [[TRUNC]] i32* [[ELEMENT]]
; CHECK-NEXT: Divergent: [Shape: Random] i32* [[GEP2:%vp.*]] = getelementptr inbounds %struct.S* %SArr i64 [[IV]] i32 0
; CHECK-NEXT: Divergent: [Shape: Random] store i32 42 i32* [[GEP2]]
; Check that uniform GEPs on StructType are uniform
; CHECK-NEXT: Uniform: [Shape: Uniform] i32* [[UNI_GEP:%vp.*]] = getelementptr inbounds %struct.S* %SArr i64 0 i32 0
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[UNI_LD:%vp.*]] = load i32* [[UNI_GEP]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[IV_ADD]] = add i64 [[IV]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[IV_CMP:%vp.*]] = icmp i64 [[IV_ADD]] i64 1024

; For sanity ensure that scatters are generated for non-uniform stores to StructType
; CHECK-LABEL: vector.body:
; CHECK: call void @llvm.masked.scatter
; CHECK: call void @llvm.masked.scatter
; For sanity ensure that scalar load is generated for uniform load to StructType in vector loop
; CHECK: {{.*}} = load i32, i32*
; CHECK-LABEL: VPlannedBB:

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, i32 }

; Function Attrs: nounwind uwtable
define dso_local void @foo(%struct.S* nocapture %SArr) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]

  ; Non-uniform GEPs
  %a = getelementptr inbounds %struct.S, %struct.S* %SArr, i64 %indvars.iv
  %a.ptr = getelementptr inbounds %struct.S, %struct.S * %a, i32 0, i32 0
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %a.ptr, align 4, !tbaa !2
  %b = getelementptr inbounds %struct.S, %struct.S* %SArr, i64 %indvars.iv, i32 0
  store i32 42, i32* %b, align 4

  ; Uniform GEP
  %c = getelementptr inbounds %struct.S, %struct.S* %SArr, i64 0, i32 0
  %c_load = load i32, i32* %c, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  store i32 1023, i32* %i.lpriv, align 4, !tbaa !7
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!2 = !{!3, !4, i64 0}
!3 = !{!"struct@S", !4, i64 0, !4, i64 4}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!4, !4, i64 0}

