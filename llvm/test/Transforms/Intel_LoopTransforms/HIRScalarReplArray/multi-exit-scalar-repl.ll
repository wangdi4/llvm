; RUN: opt -hir-ssa-deconstruction -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to scalar replace load-only groups for multi-exit loops as long as the max load is executed unconditionally.
; CHECK: Function: foo

; CHECK: + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %0 = (@A)[0][i1 + 1];
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      %1 = (@A)[0][i1];
; CHECK: |      if (%1 < 0)
; CHECK: |      {
; CHECK: |         goto for.end;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %2 = (@B)[0][i1];
; CHECK: |   (@B)[0][i1] = %2 + 1;
; CHECK: + END LOOP

; CHECK: Function: foo

; CHECK:    %scalarepl = (@A)[0][0];
; CHECK: + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %scalarepl1 = (@A)[0][i1 + 1];
; CHECK: |   %0 = %scalarepl1;
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      %1 = %scalarepl;
; CHECK: |      if (%1 < 0)
; CHECK: |      {
; CHECK: |         goto for.end;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %2 = (@B)[0][i1];
; CHECK: |   (@B)[0][i1] = %2 + 1;
; CHECK: |   %scalarepl = %scalarepl1;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv.next
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %cmp4 = icmp slt i32 %1, 0
  br i1 %cmp4, label %for.end, label %if.end

if.end:                                           ; preds = %land.lhs.true, %for.body
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx6, align 4, !tbaa !2
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %arrayidx6, align 4, !tbaa !2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %land.lhs.true, %if.end
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
