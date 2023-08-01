; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that ssa deconstruction splits the region entry block of %loop2 at the region entry intrinsic point as it also contains region exit intrinsic of %loop1.
; HIR regions are not allowed to share bblocks.

; CHECK: EntryBB: %loop2.lr.ph.split
; CHECK-NOT: @llvm.directive.region.exit
; CHECK: @llvm.directive.region.entry()

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: nounwind
declare dso_local float @expf(float) local_unnamed_addr

; Function Attrs: nounwind
declare dso_local float @sinf(float) local_unnamed_addr

; Function Attrs: nounwind uwtable
define hidden void @main(ptr nocapture %a, float %conv, float %mul9, ptr %.omp.lb, i64 %t) {
entry:
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 0, ptr %is.last, align 4
  %t0 = load i32, ptr %.omp.lb, align 4
  %cmp70 = icmp sgt i32 %t0, 16383
  br i1 %cmp70, label %exit, label %loop.ph

loop.ph:                         ; preds = %entry
  %t4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %loop1

loop1:                              ; preds = %loop1, %loop.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %loop1 ], [ 0, %loop.ph ]
  %t6 = trunc i64 %indvars.iv to i32
  %conv10 = sitofp i32 %t6 to float
  %mul11 = fmul float %mul9, %conv10
  %arrayidx13 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %t, i64 %indvars.iv
  store float %mul11, ptr %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4096
  br i1 %exitcond, label %loop2.lr.ph, label %loop1

loop2.lr.ph:                       ; preds = %loop1
  call void @llvm.directive.region.exit(token %t4) [ "DIR.OMP.END.SIMD"() ]
  %arrayidx17 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %t, i64 4096
  store float 1.000000e+00, ptr %arrayidx17, align 4
  %t7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  %mul34 = fmul float %conv, 8.000000e+00
  br label %loop2

loop2:                             ; preds = %loop2, %loop2.lr.ph
  %indvars.iv6 = phi i64 [ %indvars.iv.next7, %loop2 ], [ 0, %loop2.lr.ph ]
  %indvars.iv.next7 = add nuw nsw i64 %indvars.iv6, 1
  %arrayidx32 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %t, i64 %indvars.iv.next7
  %t8 = load float, ptr %arrayidx32, align 4
  %call = call float @expf(float %t8)
  %t9 = trunc i64 %indvars.iv6 to i32
  %conv35 = sitofp i32 %t9 to float
  %mul36 = fmul float %mul34, %conv35
  %call37 = call float @sinf(float %mul36)
  %add38 = fadd float %call, %call37
  %arrayidx42 = getelementptr inbounds [16384 x [4097 x float]], ptr %a, i64 0, i64 %t, i64 %indvars.iv6
  store float %add38, ptr %arrayidx42, align 4
  %exitcond8 = icmp eq i64 %indvars.iv.next7, 4096
  br i1 %exitcond8, label %loopexit, label %loop2

loopexit:                               ; preds = %loop2
  call void @llvm.directive.region.exit(token %t7) [ "DIR.OMP.END.SIMD"() ]
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %loop.region.exit.loopexit, %omp.inner.for.body.lr.ph
  br label %exit

exit:             ; preds = %loop.region.exit, %entry
  ret void
}

