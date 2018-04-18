; RUN: llc -mtriple=csa < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; This function has a stride of 14--don't try generating a streaming store for
; this function.
define void @Step10_outer(i64 %count, float* noalias nocapture %dxi) local_unnamed_addr #0 {
; CHECK-NOT: sst
for.body.lr.ph.split.us:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1023)
  br label %for.body.us

for.body.us:                                      ; preds = %for.body.us, %for.body.lr.ph.split.us
  %indvars.iv189 = phi i64 [ %indvars.iv.next190, %for.body.us ], [ 0, %for.body.lr.ph.split.us ]
  %clie_pse.us = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  %arrayidx84.us = getelementptr inbounds float, float* %dxi, i64 %indvars.iv189
  store float 0.000000e+00, float* %arrayidx84.us, align 4, !tbaa !2
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse.us)
  %indvars.iv.next190 = add nuw nsw i64 %indvars.iv189, 14
  %cmp2.us = icmp ult i64 %indvars.iv.next190, %count
  br i1 %cmp2.us, label %for.body.us, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %ifmerge.13
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret void
}

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.region.entry(i32) #3

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.region.exit(i32) #3

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.section.entry(i32) #3

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.section.exit(i32) #3

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { inaccessiblemem_or_argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm d69b7e153a94f84003376e93d51fd71773ace969)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
