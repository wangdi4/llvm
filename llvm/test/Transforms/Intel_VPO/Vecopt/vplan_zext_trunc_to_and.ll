;
; RUN: opt -disable-output -vplan-vec -vplan-print-after-early-peephole %s 2>&1 | FileCheck %s
;
; Check correctness of the following peehole transformation
;    %c1 = trunc i32 %t2 to i8
;    %c2 = zext i8 %c1 to i32
;  ==>
;    %c2 = and i32 t2 i32 255
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i32:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: [[VP1:%.*]] = and i32 [[VP2:%.*]] i32 255
; CHECK-NOT: trunc
; CHECK-NOT: zext

define dso_local void @_Z4initPiPli(i32* nocapture noundef writeonly %in0, i32* nocapture noundef readonly %in1, i32 noundef %N) local_unnamed_addr {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %in1, i32 %indvars.iv
  %t2 = load i32, i32* %arrayidx, align 8
  %c1 = trunc i32 %t2 to i8
  %c2 = zext i8 %c1 to i32
  %arrayidx6 = getelementptr inbounds i32, i32* %in0, i32 %indvars.iv
  store i32 %c2, i32* %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond.not = icmp eq i32 %indvars.iv.next, %N
  br i1 %exitcond.not, label %omp.loopexit, label %omp.inner.for.body

omp.loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

