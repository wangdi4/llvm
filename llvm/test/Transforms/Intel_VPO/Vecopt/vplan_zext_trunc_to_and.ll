;
; RUN: opt -disable-output -passes=vplan-vec -vplan-dump-da-shapes -vplan-print-after-early-peephole %s 2>&1 | FileCheck %s
;
; Check correctness of the following peehole transformations
;    %c1 = trunc i32 %t2 to i8
;    %c2 = zext i8 %c1 to i32
;  ==>
;    %c2 = and i32 t2 i32 255

;    %b1 = trunc i64 %t3 to i35
;    %b2 = zext i35 %b1 to i64
;  ==>
;    %b2 = and i64 t3 i64 34359738367
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i32:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: [DA: [Shape: Random]] i32 [[VP1:%.*]] = and i32 [[VP2:%.*]] i32 255
; CHECK: [DA: [Shape: Random]] i64 [[VP3:%.*]] = and i64 [[VP4:%.*]] i64 34359738367
; TODO: uncomment lines 45-49 and add a check for 128-bit values after corrections in DA.
; CHECK-NOT: trunc
; CHECK-NOT: zext

define void @_Z4initPiPli(ptr %in0, ptr %in1, ptr %in2, ptr %in3, i32 noundef %N) local_unnamed_addr {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %in1, i32 %indvars.iv
  %t2 = load i32, ptr %arrayidx, align 8
  %c1 = trunc i32 %t2 to i8
  %c2 = zext i8 %c1 to i32
  %arrayidx6 = getelementptr inbounds i32, ptr %in0, i32 %indvars.iv
  store i32 %c2, ptr %arrayidx6, align 4
  %arrayidx7 = getelementptr inbounds i64, ptr %in2, i32 %indvars.iv
  %v = sext i32 %indvars.iv to i64
  %b1 = trunc i64 %v to i35
  %b2 = zext i35 %b1 to i64
  store i64 %b2, ptr %arrayidx7, align 4
;  %arrayidx8 = getelementptr inbounds i128, i128* %in3, i32 %indvars.iv
;  %v2 = sext i32 %indvars.iv to i128
;  %b3 = trunc i128 %v2 to i67
;  %b4 = zext i67 %b3 to i128
;  store i128 %b4, i128* %arrayidx8, align 4
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

