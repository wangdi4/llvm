; RUN: llc < %s | FileCheck %s
; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @foo(double* nocapture %input_array, i32 %input_array_size) local_unnamed_addr #0 {
entry:
  %clie_pre = tail call i32 @llvm.csa.parallel.region.entry(i32 1023)
  %cmp1 = icmp sgt i32 %input_array_size, 0
  br i1 %cmp1, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = zext i32 %input_array_size to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  tail call void @llvm.csa.parallel.region.exit(i32 %clie_pre)
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %0 = trunc i64 %indvars.iv.next to i32
  %conv = sitofp i32 %0 to double
; CHECK: seqotne64 %[[indvar:[0-9a-z_]+]]
; CHECK: add32 %[[indvarinc:[0-9a-z_]+]], %[[indvar]], 1
; CHECK: cvtf64s32 %[[result:[0-9a-z_]+]], %[[indvarinc]]
  %call = tail call double @sin(double %conv) #3
  %arrayidx = getelementptr inbounds double, double* %input_array, i64 %indvars.iv
  %clie_pse = tail call i32 @llvm.csa.parallel.section.entry(i32 %clie_pre)
  store double %call, double* %arrayidx, align 8, !tbaa !2
  tail call void @llvm.csa.parallel.section.exit(i32 %clie_pse)
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind readnone
declare double @sin(double) local_unnamed_addr #1

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.region.entry(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.region.exit(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare i32 @llvm.csa.parallel.section.entry(i32) #2

; Function Attrs: inaccessiblemem_or_argmemonly nounwind
declare void @llvm.csa.parallel.section.exit(i32) #2

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
attributes #2 = { inaccessiblemem_or_argmemonly nounwind }
attributes #3 = { nounwind readnone }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (https://git.llvm.org/git/clang.git/ ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm bc1b951e08532780a71d794456cbc998a74baeba)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
