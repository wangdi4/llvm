
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:            |   (@B)[0][4 * i1] = 4 * i1 + %n;
; CHECK:            |   (@B)[0][4 * i1 + 1] = 4 * i1 + %n + 1;
; CHECK:            |   (@B)[0][4 * i1 + 2] = 4 * i1 + %n + 2;
; CHECK:            |   (@B)[0][4 * i1 + 3] = 4 * i1 + %n + 3;
; CHECK:            + END LOOP
; CHECK:      END REGION

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:            |   (@B)[0][i1] = i1 + %n;
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR; ModuleID = 'iv-pattern.c'
source_filename = "iv-pattern.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp37 = icmp sgt i32 %n, 0
  br i1 %cmp37, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %1, %n
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx, align 16, !tbaa !2
  %2 = or i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  %3 = trunc i64 %2 to i32
  %4 = add i32 %3, %n
  store i32 %4, ptr %arrayidx5, align 4, !tbaa !2
  %5 = or i64 %indvars.iv, 2
  %arrayidx10 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %5
  %6 = trunc i64 %5 to i32
  %7 = add i32 %6, %n
  store i32 %7, ptr %arrayidx10, align 8, !tbaa !2
  %8 = or i64 %indvars.iv, 3
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %8
  %9 = trunc i64 %8 to i32
  %10 = add i32 %9, %n
  store i32 %10, ptr %arrayidx15, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f51709b189b5299f1276f147872f17a299139ee2)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
