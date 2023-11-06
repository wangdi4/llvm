; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; Current implementation does not handle rerolling of reduction-like pattern.
; For this specific example, ICC does not reroll either.

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:            |   %0 = (@A)[0][2 * i1];
; CHECK:            |   %S.013 = %0 + %S.013  +  (@A)[0][2 * i1 + 1];
; CHECK:            + END LOOP
; CHECK:      END REGION

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   %0 = (@A)[0][i1];
; CHECK:            |   %S.013 = %S.013  +  %0;
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR; ModuleID = 'red-unrolled.c'
source_filename = "red-unrolled.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i64 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add4.lcssa = phi i64 [ %add4, %for.body ]
  ret i64 %add4.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.013 = phi i64 [ 0, %entry ], [ %add4, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i64], ptr @A, i64 0, i64 %indvars.iv
  %0 = load i64, ptr %arrayidx, align 16, !tbaa !2
  %add = add nsw i64 %0, %S.013
  %1 = or i64 %indvars.iv, 1
  %arrayidx3 = getelementptr inbounds [1000 x i64], ptr @A, i64 0, i64 %1
  %2 = load i64, ptr %arrayidx3, align 8, !tbaa !2
  %add4 = add nsw i64 %add, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 16fcefa05d1945cdd402c6067aa5ed458682c9a1)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_x", !4, i64 0}
!4 = !{!"long long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
