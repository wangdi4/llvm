; C Source
; void foo(int *arr)
; {
;   int i;
;
; #pragma novector
;   for (i = 0; i < 1024; i++)
;     arr[i] = i;
; }
; ModuleID = 'nv.c'
; Check that the loop is not vectorized
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec  < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; CHECK: DO i1 = 0, 1023, 1
source_filename = "nv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %arr) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 2a99f65e666939ed83d39310be8421f2dd32c6a3) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c8989f04f7ccc7b7412ec77d41e2bebcb62e0c8f)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.vectorize.width", i32 1}
