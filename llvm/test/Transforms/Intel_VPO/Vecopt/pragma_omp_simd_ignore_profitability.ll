; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
;RUN: opt < %s -passes="vplan-vec" -S | FileCheck %s


; This test checks that the code is force-vectorized when #pragma omp simd is specified on the loop.
;float arr[1024];
;float foo(int n1) {
;  int index;
;
;#pragma omp simd
;  for (index = 0; index < 1024; index++) {
;    if (arr[index] > 0) {
;      arr[index + n1] = index + n1 * n1 + 3;
;    }
;  }
;  return arr[0];
;}

; CHECK-LABEL: foo
; CHECK: %[[WideLoad:.*]] = load <{{.*}} x float>, ptr %[[GEPIndVar:.*]], align 8
; CHECK: %[[CmpRes:.*]] = fcmp ogt <{{.*}} x float> %[[WideLoad]], zeroinitializer
; CHECK: %[[TruncRes:.*]] = trunc <{{.*}} x i64> %[[VecInd:.*]] to <{{.*}} x i32>
; CHECK: %[[AddRes:.*]] = add <{{.*}} x i32> %[[BroadcastSplat1:.*]], %[[TruncRes]]
; CHECK: %[[SitOfPRes:.*]] = sitofp <{{.*}} x i32> %[[AddRes]] to <{{.*}} x float>
; CHECK: %[[AddRes1:.*]] = add nsw <{{.*}} x i32> %[[TruncRes]], %[[BroadcastSplat2:.*]]
; CHECK: %[[SExtRes:.*]] = sext <{{.*}} x i32> %[[AddRes1]] to <{{.*}} x i64>


; ModuleID = 't2.c'
source_filename = "t2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16
@"@tid.addr" = external global i32
@"@bid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local float @foo(i32 %n1) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %mul2 = mul nsw i32 %n1, %n1
  %add3 = add nuw i32 %mul2, 3
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds [1024 x float], ptr @arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = fcmp ogt float %1, 0.000000e+00
  br i1 %cmp1, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %2 = trunc i64 %indvars.iv to i32
  %add4 = add i32 %add3, %2
  %conv = sitofp i32 %add4 to float
  %add5 = add nsw i32 %2, %n1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [1024 x float], ptr @arr, i64 0, i64 %idxprom6, !intel-tbaa !2
  store float %conv, ptr %arrayidx7, align 4, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.216

DIR.OMP.END.SIMD.216:                             ; preds = %DIR.OMP.END.SIMD.2
  %3 = load float, ptr @arr, align 16, !tbaa !2
  ret float %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
; end INTEL_FEATURE_SW_ADVANCED
