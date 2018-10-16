; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not perform scalar replacement when the bitcast type of refs is different.

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %t.012 = (%A)[i1 + -1]  +  %t.012;
; CHECK: |   %0 = (float*)(%A)[i1]  +  %0;
; CHECK: |   (%p)[0] = %0;
; CHECK: + END LOOP

; CHECK-NOT: modified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32* nocapture readonly %A, float* nocapture %p) local_unnamed_addr #0 {
entry:
  %.pre = load float, float* %p, align 4, !tbaa !2
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %0 = phi float [ %.pre, %entry ], [ %add3, %for.body ]
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t.012 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add = add nsw i32 %2, %t.012
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %3 = bitcast i32* %arrayidx2 to float*
  %4 = load float, float* %3, align 4, !tbaa !2
  %add3 = fadd float %4, %0
  store float %add3, float* %p, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 128352294dcc96fea1ef64497c471e8e6af14866) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 7c11733d3668b2e82ad7039265e7e2ed205180e1)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
