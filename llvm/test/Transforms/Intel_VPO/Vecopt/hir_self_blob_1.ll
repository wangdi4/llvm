; Test for successful vectorization - test should not crash.
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -S -vplan-force-vf=4 -print-after=hir-vplan-vec 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" < %s -S -vplan-force-vf=4 2>&1 | FileCheck %s

; CHECK: DO i2 = 0, {{.*}}, 4

; ModuleID = '111.c'
source_filename = "111.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp29 = icmp sgt i32 %n, 0
  br i1 %cmp29, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv32 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next33, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %x.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %x.0.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi i32 [ %add, %for.body4 ]
  %arrayidx9 = getelementptr inbounds [1024 x i32], [1024 x i32]* @c, i64 0, i64 %indvars.iv32
  store i32 %add.lcssa, i32* %arrayidx9, align 4, !tbaa !2
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond35 = icmp eq i64 %indvars.iv.next33, %wide.trip.count
  br i1 %exitcond35, label %for.cond.cleanup.loopexit, label %for.body4.preheader

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, %n
  %add5 = add nsw i32 %add, 1
  %arrayidx7 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv
  store i32 %add5, i32* %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang d3502e2096448b81a99df35eb5758566d27dd199) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ec5c2ba25bc3213af1e1903881ec25e4e07e6b92)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

