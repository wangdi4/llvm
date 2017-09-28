; ModuleID = 'shtest.c'
; source_filename = "shtest.c"
; typedef float float8 __attribute__((ext_vector_type(8)));
; typedef float float4 __attribute__((ext_vector_type(4)));
; 
; 
; void foo(float8 *farr1, float4 *farr2)
; {
;   int index;
; 
; #pragma omp simd
;   for (index = 0; index < 1024; index++)
;     farr2[index] = farr1[index].lo;
; }
; 
; RUN: opt -VPlanDriver -S %s | FileCheck %s

; Test checks that the compilation does not assert
; CHECK: define void @foo
; CHECK: ret void
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(<8 x float>* nocapture readonly %farr1, <4 x float>* nocapture %farr2) {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds <8 x float>, <8 x float>* %farr1, i64 %indvars.iv
  %0 = load <8 x float>, <8 x float>* %arrayidx, align 32
  %1 = shufflevector <8 x float> %0, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %arrayidx2 = getelementptr inbounds <4 x float>, <4 x float>* %farr2, i64 %indvars.iv
  store <4 x float> %1, <4 x float>* %arrayidx2, align 16, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
