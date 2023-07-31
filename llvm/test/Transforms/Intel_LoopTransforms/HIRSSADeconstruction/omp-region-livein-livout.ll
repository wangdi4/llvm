; RUN: opt %s -passes="hir-ssa-deconstruction" -S | FileCheck %s

; There are two omp loops with header omp.inner.for.body6 and
; omp.inner.for.body23.
; We verify that-
; 1. omp.inner.for.body23.lr.ph which has the region exit intrinsic of first
;    loop and reigon entry intrinsic of second loop is split into different
;    blocks.
; 2. A single operand phi copy is created for %conv which is liveout from first
;    region.

; CHECK: call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
; CHECK-NEXT: br label %omp.inner.for.body23.lr.ph.split.split

; CHECK: omp.inner.for.body23.lr.ph.split.split:
; CHECK-NEXT: %liveoutcopy = phi float [ %conv, %omp.inner.for.body23.lr.ph ]

; liveout use of %conv should be replaced by the copy.
; CHECK: %mul33 = fmul float %liveoutcopy, 8.000000e+00

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define hidden void @main.DIR.OMP.PARALLEL.LOOP.3(ptr nocapture %a, i64 %init) {
entry:
  br label %omp.inner.for.body6.lr.ph

omp.inner.for.body6.lr.ph:                        ; preds = %entry
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %t5 = trunc i64 %init to i32
  %conv = sitofp i32 %t5 to float
  %mul9 = fmul float %conv, 0x3F40624DE0000000
  br label %omp.inner.for.body6

omp.inner.for.body6:                              ; preds = %omp.inner.for.body6, %omp.inner.for.body6.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body6 ], [ 0, %omp.inner.for.body6.lr.ph ]
  %t6 = trunc i64 %indvars.iv to i32
  %conv10 = sitofp i32 %t6 to float
  %mul11 = fmul float %mul9, %conv10
  %arrayidx13 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %init, i64 %indvars.iv
  store float %mul11, ptr %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %omp.inner.for.body23.lr.ph, label %omp.inner.for.body6

omp.inner.for.body23.lr.ph:                       ; preds = %omp.inner.for.body6
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  %arrayidx17 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %init, i64 4096
  store float 1.000000e+00, ptr %arrayidx17, align 4
  %t7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %mul33 = fmul float %conv, 8.000000e+00
  br label %omp.inner.for.body23

omp.inner.for.body23:                             ; preds = %omp.inner.for.body23, %omp.inner.for.body23.lr.ph
  %indvars.iv6 = phi i64 [ %indvars.iv.next7, %omp.inner.for.body23 ], [ 0, %omp.inner.for.body23.lr.ph ]
  %indvars.iv.next7 = add nuw nsw i64 %indvars.iv6, 1
  %arrayidx31 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %init, i64 %indvars.iv.next7
  %t8 = load float, ptr %arrayidx31, align 4
  %call = call float @expf(float %t8)
  %t9 = trunc i64 %indvars.iv6 to i32
  %conv34 = sitofp i32 %t9 to float
  %mul35 = fmul float %mul33, %conv34
  %call36 = call float @sinf(float %mul35)
  %add37 = fadd float %call, %call36
  %arrayidx41 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %init, i64 %indvars.iv6
  store float %add37, ptr %arrayidx41, align 4
  %exitcond8 = icmp eq i64 %indvars.iv.next7, 4096
  br i1 %exitcond8, label %DIR.OMP.END.SIMD.6, label %omp.inner.for.body23

DIR.OMP.END.SIMD.6:                               ; preds = %omp.inner.for.body23
  call void @llvm.directive.region.exit(token %t7) [ "DIR.OMP.END.SIMD"() ]
  br label %loop.region.exit.loopexit

loop.region.exit.loopexit:                        ; preds = %DIR.OMP.END.SIMD.6
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
declare dso_local float @expf(float) local_unnamed_addr

; Function Attrs: nounwind
declare dso_local float @sinf(float) local_unnamed_addr

