; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'prefetch.c'
source_filename = "prefetch.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define double @pretest(double* nocapture readonly %addr) local_unnamed_addr #0 {
; CSA_CHECK-label: pretest
; CSA_CHECK: prefetch
; CSA_CHECK-SAME: MEMLEVEL_T0
; CSA_CHECK: prefetch
; CSA_CHECK-SAME: MEMLEVEL_T1
; CSA_CHECK: prefetch
; CSA_CHECK-SAME: MEMLEVEL_T2
; CSA_CHECK: prefetch
; CSA_CHECK-SAME: MEMLEVEL_NTA
; CSA_CHECK: prefetchw
; CSA_CHECK-SAME: MEMLEVEL_T0
; CSA_CHECK: prefetchw
; CSA_CHECK-SAME: MEMLEVEL_T1
; CSA_CHECK: prefetchw
; CSA_CHECK-SAME: MEMLEVEL_T2
; CSA_CHECK: prefetchw
; CSA_CHECK-SAME: MEMLEVEL_NTA
entry:
  %0 = bitcast double* %addr to i8*
  tail call void @llvm.prefetch(i8* %0, i32 0, i32 3, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 0, i32 2, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 0, i32 1, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 0, i32 0, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 1, i32 3, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 1, i32 2, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 1, i32 1, i32 1)
  tail call void @llvm.prefetch(i8* %0, i32 1, i32 0, i32 1)
  %1 = load double, double* %addr, align 8, !tbaa !2
  ret double %1
}

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.prefetch(i8* nocapture readonly, i32, i32, i32) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { inaccessiblemem_or_argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-clang c46312bbdaea214a8c58135cb2abc778e7336b49) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm f08844d253877b60beeb73d9551c6b4ff9eda55d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
