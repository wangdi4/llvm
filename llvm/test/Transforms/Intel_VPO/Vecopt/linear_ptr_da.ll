; Test DA divergence and vector shape propagation for a simple linear pointer example.
; For this case, the gep indices are constant, but the ptr is unit stride.

; REQUIRES: asserts
; RUN: opt -S %s -VPlanDriver -disable-vplan-da=false -vplan-loop-cfu -debug 2>&1 | FileCheck %s

; CHECK: Basic Block: {{BB[0-9]+}}
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i32 1] i32 [[VAL1:%vp.*]] = phi  [ i32 0, {{BB[0-9]+}} ],  [ i32 [[VAL2:%vp.*]], {{BB[0-9]+}} ]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i32* [[VAL3:%vp.*]] = phi  [ i32* %p, {{BB[0-9]+}} ],  [ i32* [[VAL4:%vp.*]], {{BB[0-9]+}} ]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i32 1] store i32 [[VAL1]] i32* [[VAL3]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride Pointer, Stride: i64 4] i32* [[VAL4]] = getelementptr inbounds i32* [[VAL3]] i64 1
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i32 [[VAL2]] = add i32 [[VAL1]] i32 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 {{%vp.*}} = icmp i32 [[VAL2]] i32 256

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %p) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
