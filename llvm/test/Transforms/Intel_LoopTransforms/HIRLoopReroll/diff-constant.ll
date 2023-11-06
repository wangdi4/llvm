
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Different RHSs.

; CHECK: Function: foo

; CHECK: <0>       BEGIN REGION { }
; CHECK: <19>            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK: <3>             |   (@A)[0][4 * i1] = 3;
; CHECK: <6>             |   (@A)[0][4 * i1 + 1] = 4;
; CHECK: <9>             |   (@A)[0][4 * i1 + 2] = 5;
; CHECK: <12>            |   (@A)[0][4 * i1 + 3] = 6;
; CHECK: <19>            + END LOOP
; CHECK: <0>       END REGION

; CHECK: Function: foo

; CHECK: <0>       BEGIN REGION { }
; CHECK: <19>            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK: <3>             |   (@A)[0][4 * i1] = 3;
; CHECK: <6>             |   (@A)[0][4 * i1 + 1] = 4;
; CHECK: <9>             |   (@A)[0][4 * i1 + 2] = 5;
; CHECK: <12>            |   (@A)[0][4 * i1 + 3] = 6;
; CHECK: <19>            + END LOOP
; CHECK: <0>       END REGION

;Module Before HIR; ModuleID = 'new-constant.c'
source_filename = "new-constant.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %n, 0
  br i1 %cmp19, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 3, ptr %arrayidx, align 16, !tbaa !2
  %1 = or i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %1
  store i32 4, ptr %arrayidx2, align 4, !tbaa !2
  %2 = or i64 %indvars.iv, 2
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %2
  store i32 5, ptr %arrayidx5, align 8, !tbaa !2
  %3 = or i64 %indvars.iv, 3
  %arrayidx8 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %3
  store i32 6, ptr %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 498c08595872ebc415ef62b88f071225808af23b)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
