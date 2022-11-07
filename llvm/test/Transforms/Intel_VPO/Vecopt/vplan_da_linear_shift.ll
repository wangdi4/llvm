; RUN: opt < %s -vplan-vec -vplan-dump-da -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes="vplan-vec" -vplan-dump-da -S 2>&1 | FileCheck %s

; Test stride for shl instruction. It should be stride of 2 and not random. 

; CHECK: [Shape: Strided, Stride: i64 2] i64 %[[VP0:.*]] = shl

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z3fooPl(i64* nocapture noundef writeonly %a) local_unnamed_addr #0 {
DIR.OMP.SIMD.114:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.114
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 2) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %2 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %2
  store i64 %2, i64* %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
