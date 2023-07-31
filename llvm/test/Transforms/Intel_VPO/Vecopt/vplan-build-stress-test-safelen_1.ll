; Test that we can build VPlan in stress test mode for a simd loop marked with safelen(1)
; RUN: opt -S -passes="vplan-vec" -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; REQUIRES: asserts
; CHECK: Vectorization Plan{{.*}} Plain CFG
; CHECK-LABEL: @foo(
; CHECK: ret
define void @foo(ptr %ip, i32 %n) {
entry:
  %cmp = icmp sgt i32 %n, 0
  %wide.trip.count = sext i32 %n to i64
  br i1 %cmp, label %DIR.OMP.SIMD.112, label %omp.precond.end

DIR.OMP.SIMD.112:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SAFELEN"(i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.112
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.112 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
