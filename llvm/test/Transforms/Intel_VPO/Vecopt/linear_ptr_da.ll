; Test DA divergence and vector shape propagation for a simple linear pointer example.
; For this case, the gep indices are constant, but the ptr is unit stride.

; REQUIRES: asserts
; RUN: opt -VPlanDriver -debug-only=vplan-divergence-analysis -vplan-force-vf=2 %s 2>&1 | FileCheck %s

; CHECK: Basic Block: {{BB[0-9]+}}
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[VAL1:%vp.*]] = phi  [ i32 0, {{BB[0-9]+}} ],  [ i32 [[VAL2:%vp.*]], {{BB[0-9]+}} ]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[VAL3:%vp.*]] = phi  [ i32* %p, {{BB[0-9]+}} ],  [ i32* [[VAL4:%vp.*]], {{BB[0-9]+}} ]
; CHECK-NEXT: Divergent: [Shape: Random] store i32 [[VAL1]] i32* [[VAL3]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[VAL4]] = getelementptr inbounds i32* [[VAL3]] i64 1
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i32 [[VAL2]] = add i32 [[VAL1]] i32 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 {{%vp.*}} = icmp i32 [[VAL2]] i32 256

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %p) {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %.omp.iv.0 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add1, %omp.inner.for.body ]
  %p.addr.06 = phi i32* [ %p, %omp.inner.for.body.lr.ph ], [ %incdec.ptr, %omp.inner.for.body ]
  store i32 %.omp.iv.0, i32* %p.addr.06, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.06, i64 1
  %add1 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i32 %add1, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; This test checks that pointer-inductions are correctly identified when different step-sizes are used.

; CHECK: Divergent: [Shape: Strided, Stride: i64 3] i32 [[IV:%.*]] = phi  [ i32 0, {{BB[0-9]+}} ],  [ i32 [[ADD2:%.*]], {{BB[0-9]+}} ]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] i32* [[PHI1:%.*]] = phi  [ i32* %p1, {{BB[0-9]+}} ],  [ i32* [[INC2:%.*]], {{BB[0-9]+}} ]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] i32* [[PHI2:%.*]] = phi  [ i32* %p2, {{BB[0-9]+}} ],  [ i32* [[INC3:%.*]], {{BB[0-9]+}} ]
; CHECK: Divergent: [Shape: Strided, Stride: i64 8] i32* [[INC1:%.*]] = getelementptr inbounds i32* [[PHI1]] i64 1
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 8] i32* [[INC2]] = getelementptr inbounds i32* [[INC1]] i64 1
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 8] i32* [[INC3]] = getelementptr inbounds i32* [[PHI2]] i64 2
; CHECK: Divergent: [Shape: Strided, Stride: i64 3] i32 [[ADD1:%.*]] = add i32 [[IV]] i32 1
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 3] i32 [[ADD2]] = add i32 [[ADD1]] i32 2
; CHECK: Divergent: [Shape: Strided, Stride: i64 12] i32* [[INC4:%.*]] = getelementptr inbounds i32* %p3 i32 [[ADD2]]


; Function Attrs: nounwind uwtable
define dso_local void @foo2(i32* %p1, i32* %p2, i32* %p3) {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %iv = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add2, %omp.inner.for.body ]
  %p1.addr = phi i32* [ %p1, %omp.inner.for.body.lr.ph ], [ %incptr2, %omp.inner.for.body ]
  %p2.addr = phi i32* [ %p2, %omp.inner.for.body.lr.ph ], [ %incptr3, %omp.inner.for.body ]
  store i32 %iv, i32* %p1.addr, align 4
  %incptr1 = getelementptr inbounds i32, i32* %p1.addr, i64 1
  %incptr2 = getelementptr inbounds i32, i32* %incptr1, i64 1
  %incptr3 = getelementptr inbounds i32, i32* %p2.addr, i64 2
  %add1 = add nuw nsw i32 %iv, 1
  %add2 = add nuw nsw i32 %add1, 2
  %incptr4 = getelementptr inbounds i32, i32* %p3, i32 %add2
  %exitcond = icmp eq i32 %add2, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
