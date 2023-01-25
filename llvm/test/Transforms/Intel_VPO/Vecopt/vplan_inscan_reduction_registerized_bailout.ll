; Test to verify that VPlan vectorizer bails out on registerized inscan
; reduction. With the memory guards enabled for inscan reduction, it is an
; unexpected case.

; RUN: opt -disable-output -passes="vplan-vec" -vplan-force-vf=2 -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK: LVP: Registerized UDR/Scan found.
; CHECK: LVP: VPlan is not legal to process, bailing out.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @_Z3fooPfS_i(ptr %A, ptr %B, i32 %N) {
entry:
  %x.red = alloca float, align 4
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store float 1.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.136

DIR.OMP.SIMD.136:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red, float zeroinitializer, i32 1, i64 1) ]
  br label %DIR.OMP.SIMD.137

DIR.OMP.SIMD.137:                                 ; preds = %DIR.OMP.SIMD.136
  %wide.trip.count = zext i32 %N to i64
  %.pre = load float, ptr %x.red, align 4
  br label %DIR.OMP.END.SCAN.335

DIR.OMP.END.SCAN.335:                             ; preds = %DIR.OMP.SIMD.137, %DIR.OMP.END.SCAN.3
  %red.phi = phi float [ %.pre, %DIR.OMP.SIMD.137 ], [ %red.ld, %DIR.OMP.END.SCAN.3 ]
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.137 ], [ %indvars.iv.next, %DIR.OMP.END.SCAN.3 ]
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %a.ld = load float, ptr %arrayidx, align 4
  %add5 = fadd fast float %red.phi, %a.ld
  store float %add5, ptr %x.red, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.END.SCAN.335
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(ptr %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.5
  %red.ld = load float, ptr %x.red, align 4
  %arrayidx7 = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  store float %red.ld, ptr %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.7, label %DIR.OMP.END.SCAN.335

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SCAN.3
  %.lcssa = phi float [ %red.ld, %DIR.OMP.END.SCAN.3 ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.1, %entry
  %x.1 = phi float [ 1.000000e+00, %entry ], [ %.lcssa, %DIR.OMP.END.SIMD.1 ]
  ret float %x.1
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
