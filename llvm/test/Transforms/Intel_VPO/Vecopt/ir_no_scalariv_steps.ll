; ModuleID = 'il1.c'
; source_filename = "il1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; void foo(long *ip)
; {
;   long index;
;
; #pragma omp simd simdlen(4)
;   for (index = 0; index < 1024; index++)
;     ip[index] = index;
; }
; RUN: opt -VPlanDriver -S %s | FileCheck %s

; This test checks that scalar IV steps are not generated in vector loop
; CHECK: vector.ph:
; CHECK: vector.body:
; CHECK:   %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
; CHECK-NOT: {{.*}} = add i64 %index, {{[0123]}}
; CHECK: {{.*}} = getelementptr inbounds i64, i64* %ip, i64 %index
; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %ip)  {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %entry
  %.omp.iv.07 = phi i64 [ 0, %entry ], [ %add1, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %ip, i64 %.omp.iv.07
  store i64 %.omp.iv.07, i64* %arrayidx, align 8, !tbaa !1
  %add1 = add nuw nsw i64 %.omp.iv.07, 1
  %exitcond = icmp eq i64 %add1, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21478)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
