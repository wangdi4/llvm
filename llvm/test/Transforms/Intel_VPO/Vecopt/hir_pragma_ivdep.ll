; C Source:
; int arr[1024];
;
; void foo(int n, int n2)
; {
;   int i;
;
; #pragma ivdep
;   for (i = 0; i < 1024; i++)
;     arr[i] = arr[i + n] * n2;
; }
; ModuleID = 'tivdep.c'
; Check that the loop is vectorized
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -print-before=VPlanDriverHIR -print-after=VPlanDriverHIR  < %s 2>&1 | FileCheck %s
; CHECK: + DO i1 = 0, 1023, 1   <DO_LOOP> <ivdep>
; CHECK: + END LOOP

; CHECK: + DO i1 = 0, 1023, 4   <DO_LOOP> <novectorize> <ivdep>
; CHECK: + END LOOP

; Test fails pending on DD analysis changes to consume ivdep information.
;
source_filename = "tivdep.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(i32 %n, i32 %n2) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = add nsw i64 %indvars.iv, %0
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %mul = mul nsw i32 %2, %n2
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  store i32 %mul, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 2a99f65e666939ed83d39310be8421f2dd32c6a3) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c8989f04f7ccc7b7412ec77d41e2bebcb62e0c8f)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.vectorize.ivdep_back"}
