; RUN: opt -disable-output -passes=vplan-vec -vplan-force-vf=2 -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-dump-induction-init-details < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test makes sure iv range analyis during induction importing can handle ptr inductions.
; There is no such support yet, but this test should compile cleanly and can later be used
; to add support when/if required.

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; CHECK: PtrInduction(+) Start: ptr %b Step: i64 4 StartVal: ? EndVal: ?
; CHECK: induction-init{getelementptr, StartVal: ?, EndVal: ?} ptr %b i64 4

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %a, ptr nocapture readonly %b) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %ptr.011 = phi ptr [ %b, %DIR.OMP.SIMD.1 ], [ %incdec.ptr, %omp.inner.for.body ]
  %1 = load i32, ptr %ptr.011, align 4
  %ptridx1 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %1, ptr %ptridx1, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %ptr.011, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}
