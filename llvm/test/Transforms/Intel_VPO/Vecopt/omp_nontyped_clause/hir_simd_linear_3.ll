; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -disable-output -debug-only=LoopVectorizationPlannerHIR < %s 2>&1 | FileCheck %s

; CHECK: LVP: Pointer induction currently not supported.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; *** IR Dump Before vpo::VPlanDriverHIRPass ***
; <0>          BEGIN REGION { }
; <2>                %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR(&((%lp.addr.linear)[0])2),  QUAL.OMP.LINEAR:IV(&((%l1.linear.iv)[0])1),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <19>
; <19>               + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
; <6>                |   @_Z3bazPPl(&((%lp.addr.linear)[0]));
; <7>                |   %1 = (%lp.addr.linear)[0];
; <9>                |   (%lp.addr.linear)[0] = &((%1)[2]);
; <19>               + END LOOP
; <19>
; <17>               @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>          END REGION

define void @_Z3fooPl(i64* noundef %lp) {
DIR.OMP.SIMD.1:
  %lp.addr.linear = alloca i64*, align 8
  %l1.linear.iv = alloca i64, align 8
  store i64* %lp, i64** %lp.addr.linear, align 8
  br label %DIR.OMP.SIMD.116

DIR.OMP.SIMD.116:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i64** %lp.addr.linear, i32 2), "QUAL.OMP.LINEAR:IV"(i64* %l1.linear.iv, i32 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.116
  %.omp.iv.local.010 = phi i64 [ 0, %DIR.OMP.SIMD.116 ], [ %add1, %omp.inner.for.body ]
  call void @_Z3bazPPl(i64** noundef nonnull %lp.addr.linear)
  %1 = load i64*, i64** %lp.addr.linear, align 8
  %add.ptr = getelementptr inbounds i64, i64* %1, i64 2
  store i64* %add.ptr, i64** %lp.addr.linear, align 8
  %add1 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond.not = icmp eq i64 %add1, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.115, label %omp.inner.for.body

DIR.OMP.END.SIMD.115:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.115
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @_Z3bazPPl(i64** noundef) #0

attributes #0 = { nounwind }
