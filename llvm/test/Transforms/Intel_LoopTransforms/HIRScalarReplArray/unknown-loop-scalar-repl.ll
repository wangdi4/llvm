; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to scalar replace load-only groups for unknown loops.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   %1 = (@A)[0][i1 + 1];
; CHECK: |   (@B)[0][%i.013] = %0 + %1;
; CHECK: |   %i.013 = %i.013  <<  1;
; CHECK: |   if (%i.013 < %n)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK:    %scalarepl = (@A)[0][0];
; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %0 = %scalarepl;
; CHECK: |   %scalarepl1 = (@A)[0][i1 + 1];
; CHECK: |   %1 = %scalarepl1;
; CHECK: |   (@B)[0][%i.013] = %0 + %1;
; CHECK: |   %i.013 = %i.013  <<  1;
; CHECK: |   if (%i.013 < %n)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      %scalarepl = %scalarepl1;
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %n, 1
  br i1 %cmp12, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %i.013 = phi i32 [ %mul, %for.body ], [ 1, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %add3 = add nsw i32 %1, %0
  %2 = zext i32 %i.013 to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %2
  store i32 %add3, i32* %arrayidx5, align 4, !tbaa !2
  %mul = shl nsw i32 %i.013, 1
  %cmp = icmp slt i32 %mul, %n
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6b6cced55129613697c3a5b9d8270163bedefc32) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a38fac58acf8e41e093f119a9a0db56e74cc2dd6)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
