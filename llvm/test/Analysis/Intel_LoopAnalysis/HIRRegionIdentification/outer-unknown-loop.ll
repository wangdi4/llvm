; RUN: opt < %s -passes='print<hir-region-identification>' -xmain-opt-level=3 -disable-output 2>&1 | FileCheck %s --check-prefix=O3
; RUN: opt < %s -passes='print<hir-region-identification>' -hir-cost-model-throttling=0 -disable-output 2>&1 | FileCheck %s --check-prefix=ALL

; O3: EntryBB: %for.body3

; ALL: EntryBB: %for.cond1.preheader

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n, ptr nocapture %A) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %n, 1
  br i1 %cmp12, label %for.cond1.preheader.preheader, label %for.end5

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc4
  %i.013 = phi i32 [ %mul, %for.inc4 ], [ 1, %for.cond1.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, %i.013
  store i32 %add, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc4, label %for.body3

for.inc4:                                         ; preds = %for.body3
  %mul = shl nsw i32 %i.013, 1
  %cmp = icmp slt i32 %mul, %n
  br i1 %cmp, label %for.cond1.preheader, label %for.end5.loopexit

for.end5.loopexit:                                ; preds = %for.inc4
  br label %for.end5

for.end5:                                         ; preds = %for.end5.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5f4f662867240aabc27bc9491b73d220049214d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 18c1afa0466f2dec43143360dc641c9fc7a5ab87)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
