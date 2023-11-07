; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Blob is different, in the second store out of fours.

; CHECK: Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:            |   %1 = (@C)[0][4 * i1];
; CHECK:            |   (@B)[0][4 * i1] = %1;
; CHECK:            |   (@A)[0][4 * i1] = (%n * %n * %1);
; CHECK:            |   %3 = (@C)[0][4 * i1 + 1];
; CHECK:            |   (@B)[0][4 * i1 + 1] = %3;
; CHECK:            |   (@A)[0][4 * i1 + 1] = (%div * %3);
; CHECK:            |   %5 = (@C)[0][4 * i1 + 2];
; CHECK:            |   (@B)[0][4 * i1 + 2] = %5;
; CHECK:            |   (@A)[0][4 * i1 + 2] = (%n * %n * %5);
; CHECK:            |   %7 = (@C)[0][4 * i1 + 3];
; CHECK:            |   (@B)[0][4 * i1 + 3] = %7;
; CHECK:            |   (@A)[0][4 * i1 + 3] = (%n * %n * %7);
; CHECK:            + END LOOP
; CHECK:      END REGION

; CHECK: Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:            |   %1 = (@C)[0][4 * i1];
; CHECK:            |   (@B)[0][4 * i1] = %1;
; CHECK:            |   (@A)[0][4 * i1] = (%n * %n * %1);
; CHECK:            |   %3 = (@C)[0][4 * i1 + 1];
; CHECK:            |   (@B)[0][4 * i1 + 1] = %3;
; CHECK:            |   (@A)[0][4 * i1 + 1] = (%div * %3);
; CHECK:            |   %5 = (@C)[0][4 * i1 + 2];
; CHECK:            |   (@B)[0][4 * i1 + 2] = %5;
; CHECK:            |   (@A)[0][4 * i1 + 2] = (%n * %n * %5);
; CHECK:            |   %7 = (@C)[0][4 * i1 + 3];
; CHECK:            |   (@B)[0][4 * i1 + 3] = %7;
; CHECK:            |   (@A)[0][4 * i1 + 3] = (%n * %n * %7);
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR; ModuleID = 'negative.c'
source_filename = "negative.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %div = sdiv i32 %n, 2
  %cmp72 = icmp sgt i32 %n, 0
  br i1 %cmp72, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 16, !tbaa !2
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %1, ptr %arrayidx2, align 16, !tbaa !2
  %mul5 = mul nsw i32 %1, %mul
  %arrayidx7 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %mul5, ptr %arrayidx7, align 16, !tbaa !2
  %2 = or i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx9, align 4, !tbaa !2
  %arrayidx12 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  store i32 %3, ptr %arrayidx12, align 4, !tbaa !2
  %mul16 = mul nsw i32 %3, %div
  %arrayidx19 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %2
  store i32 %mul16, ptr %arrayidx19, align 4, !tbaa !2
  %4 = or i64 %indvars.iv, 2
  %arrayidx22 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %4
  %5 = load i32, ptr %arrayidx22, align 8, !tbaa !2
  %arrayidx25 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %4
  store i32 %5, ptr %arrayidx25, align 8, !tbaa !2
  %mul29 = mul nsw i32 %5, %mul
  %arrayidx32 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %4
  store i32 %mul29, ptr %arrayidx32, align 8, !tbaa !2
  %6 = or i64 %indvars.iv, 3
  %arrayidx35 = getelementptr inbounds [10 x i32], ptr @C, i64 0, i64 %6
  %7 = load i32, ptr %arrayidx35, align 4, !tbaa !2
  %arrayidx38 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %6
  store i32 %7, ptr %arrayidx38, align 4, !tbaa !2
  %mul42 = mul nsw i32 %7, %mul
  %arrayidx45 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %6
  store i32 %mul42, ptr %arrayidx45, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm b025254208da19de9ee9564978a604b1b8acb28f)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
