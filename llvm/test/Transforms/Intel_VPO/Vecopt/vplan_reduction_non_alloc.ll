;
; Test to verify that VPlan vectorizer does not crash on reduction which
; is passed by a non-alloca pointer.
;
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -disable-output -vplan-print-after-vpentity-instrs %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=2 -disable-output -vplan-print-after-vpentity-instrs %s  2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: i32 [[VP_LD:%.*]] = load ptr %r.red.ptr
; CHECK: i32 [[VP_RED_INIT:%.*]] = reduction-init i32 0 i32 [[VP_LD]]
;
; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture readonly %_C, ptr %r.red.ptr) local_unnamed_addr {
DIR.OMP.SIMD.0:
  %i.linear.iv.ptr = alloca i32, align 4
  br label %omp.simd.region.entry

omp.simd.region.entry:                            ; preds = %DIR.OMP.SIMD.0
%i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %r.red.ptr, i32 zeroinitializer, i32 1),  "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv.ptr, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.simd.region.entry
  %c.ptr = load ptr, ptr %_C, align 8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %iv.next, %omp.inner.for.body ]
  store i32 %iv, ptr %i.linear.iv.ptr, align 4
  %arrayidx = getelementptr inbounds float, ptr %c.ptr, i32 %iv
  %fval = load float, ptr %arrayidx, align 4
  %fcmp = fcmp fast oeq float %fval, 1.000000e+00
  %conv = zext i1 %fcmp to i32
  %red.local = load i32, ptr %r.red.ptr, align 1
  %add1 = add i32 %red.local, %conv
  store i32 %add1, ptr %r.red.ptr, align 1
  %iv.next = add nuw nsw i32 %iv, 1
  %exitcond = icmp eq i32 %iv.next, 10
  br i1 %exitcond, label %omp.simd.loop.exit, label %omp.inner.for.body

omp.simd.loop.exit:                               ; preds = %omp.inner.for.body
  br label %omp.simd.region.exit

omp.simd.region.exit:                             ; preds = %omp.simd.loop.exit
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD

DIR.OMP.END.SIMD:                                 ; preds = %omp.simd.region.exit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

