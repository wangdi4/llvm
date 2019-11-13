; This test makes sure that the divergence/uniformity property for bitcast instruction is
; correctly inferred.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


;RUN: opt -S -VPlanDriver -enable-vp-value-codegen  -disable-vplan-codegen -debug-only=vplan-divergence-analysis %s 2>& 1| FileCheck %s

; REQUIRES:asserts

; CHECK: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[PHI:%.*]] = phi
; CHECK: Divergent: [Shape: Unit Stride, Stride: i32 1] i64 [[SEXT:%.*]] = sext i32 [[PHI]] to i64
; CHECK: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[ADD1:%.*]] = add i64 [[SEXT]] i64 1
; CHECK: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[ADD2:%.*]] = add i64 [[SEXT]] i64 2
; CHECK: Uniform: [Shape: Uniform] i32* [[GEP1:%.*]] = getelementptr inbounds [1024 x i32]* @arr2 i64 0 i64 1
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i32* [[GEP2:%.*]] = getelementptr inbounds [1024 x i32]* @arr2 i64 0 i64 [[SEXT]]

;; Check that the divergence and shape information for copy (via assignment) which can be generated along the HIR path
;; is correctly propagated.
; CHECK: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[PHI_COPY:%.*]] = bitcast i32 [[PHI]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i32* [[GEP3:%.*]] = getelementptr inbounds [1024 x i32]* @arr2 i64 0 i64 [[ADD1]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i32* [[GEP4:%.*]] = getelementptr inbounds [1024 x i32]* @arr2 i64 0 i64 [[ADD2]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] i64* [[GEP6:%.*]] = getelementptr inbounds [1024 x i64]* @arr1 i64 0 i64 [[SEXT]]
; CHECK: Divergent: [Shape: Random] <3 x i32*> [[VEC_PTR:%.*]] = insertelement <3 x i32*> {{.*}} i32* {{.*}}  i32 2
; CHECK: Uniform: [Shape: Uniform] i8* {{.*}} = bitcast i32* [[GEP1]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i8* {{.*}} = bitcast i32* [[GEP2]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] i64* {{.*}} = bitcast i32* [[GEP2]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 4] <3 x i32>* {{.*}} = bitcast i32* [[GEP2]]
; CHECK: Divergent: [Shape: Random] <3 x i64*> {{.*}} = bitcast <3 x i32*> [[VEC_PTR]]
; CHECK: Divergent: [Shape: Random] float {{.*}} = bitcast i32 [[IV:%.*]]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] double* {{.*}} = bitcast i64* [[GEP6]]

;; Check that the divergence and shape information for copy (via assignment) which can be generated along the HIR path
;; is correctly propagated.
; CHECK: Divergent: [Shape: Random] i64 [[LOAD1:%.*]] = load i64* [[GEP6]]
; CHECK: Divergent: [Shape: Random] i64 [[BC6:%.*]] = bitcast i64 [[LOAD1]]


@arr1 = common dso_local local_unnamed_addr global [1024 x i64] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
define i32 @test_bitcast_stride(i1 %flag1, i32 %val) {
omp.inner.for.body.lr.ph:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() ["DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null)]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  br label %omp.inner.for.body

omp.inner.for.body:
  %iv = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %iv.next, %omp.inner.for.body ]
  %idxprom = sext i32 %iv to i64
  %idxprom1 = add i64 %idxprom, 1
  %idxprom2 = add i64 %idxprom, 2
  %idx0 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 1
  %idx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %idxprom
  ;; Variable copy.
  %bc.val1 = bitcast i32 %iv to i32
  %idx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %idxprom1
  %idx3 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %idxprom2
  %idx5 = getelementptr inbounds [1024 x i64], [1024 x i64]* @arr1, i64 0, i64 %idxprom
  %vec1 = insertelement <3 x i32*> undef, i32* %idx1, i32 0
  %vec2 = insertelement <3 x i32*> %vec1, i32*  %idx2, i32 1
  %vec3 = insertelement <3 x i32*> %vec2, i32*  %idx3, i32 2
  %bc0 = bitcast i32* %idx0 to i8*
  %bc1 = bitcast i32* %idx1 to i8*
  %bc2 = bitcast i32* %idx1 to i64*
  %bc3 = bitcast i32* %idx1 to <3 x i32>*
  %bcVecPtr = bitcast <3 x i32*> %vec3 to <3 x i64*>
  %bc5 = bitcast i32 %iv to float
  %bc6 = bitcast i64* %idx5 to double*
  %load2 = load i64, i64* %idx5, align 4
  ;; Variable copy.
  %bc.val2 = bitcast i64 %load2 to i64
  %iv.next = add nuw nsw i32 %iv, 1
  %exitcond = icmp eq i32 %iv.next, 4096
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:
  ret i32 %val
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
