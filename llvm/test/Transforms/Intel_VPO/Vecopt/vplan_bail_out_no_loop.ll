; Test that vectorizer does not crash when the loop within SIMD region is optimized out by preceding optimizations.

; Example C code:
; void foo(char *a, char x) {
;   int i;
; #pragma omp simd
; #pragma unroll 2
;   for (i = 0; i < 2; ++i) {
;     a[i] = i;
;   }
; }

; Incoming HIR (for VPlanDriverHIR)
; <0>      BEGIN REGION { modified }
; <2>            %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ] 
; <23>          (%a)[0] = 0;
; <9>           (%a)[1] = 1;
; <19>          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ] 
; <0>      END REGION

; REQUIRES: asserts

; Check for HIR driver
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-vplan-vec -debug-only=vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR
; CHECK-HIR-LABEL: VPlan HIR Driver for Function
; CHECK-HIR: VPLAN_OPTREPORT: Loop was optimized out. 

; Check for LLVM-IR driver
; RUN: opt -loop-unroll -vplan-vec -debug-only=vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVM
; CHECK-LLVM-LABEL: VPlan LLVM-IR Driver for Function
; CHECK-LLVM: VPLAN_OPTREPORT: Loop was optimized out. 


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i8* nocapture %a, i8 signext %x) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %.omp.iv.0 = phi i32 [ %add1, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %conv = trunc i32 %.omp.iv.0 to i8
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  store i8 %conv, i8* %arrayidx, align 1, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add1 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body, !llvm.loop !5

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.unroll.count", i32 2}
