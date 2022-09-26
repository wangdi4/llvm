; Test for successful vectorization - test should not crash.
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -S -vplan-force-vf=4 -print-after=hir-vplan-vec 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" < %s -S -vplan-force-vf=4 2>&1 | FileCheck %s

; CHECK: DO i2 = {{.*}}, {{.*}}, 4
; ModuleID = 'mod_06.c'
source_filename = "mod_06.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = dso_local local_unnamed_addr global [2 x i32] [i32 1, i32 1], align 4
@a = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %m1, i32 %m2) local_unnamed_addr #0 {
entry:
  %cmp28 = icmp sgt i32 %m1, 10
  br i1 %cmp28, label %for.body.lr.ph, label %for.end11

for.body.lr.ph:                                   ; preds = %entry
  %cmp326 = icmp sgt i32 %m2, 0
  %0 = add nsw i32 %m2, 4
  %1 = sext i32 %0 to i64
  %wide.trip.count = sext i32 %m1 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc9, %for.body.lr.ph
  %indvars.iv32 = phi i64 [ 10, %for.body.lr.ph ], [ %indvars.iv.next33, %for.inc9 ]
  %n.031 = phi i32 [ 0, %for.body.lr.ph ], [ %n.1.lcssa, %for.inc9 ]
  %2 = trunc i64 %indvars.iv32 to i32
  %add = add nsw i32 %n.031, %2
  br i1 %cmp326, label %for.body4.preheader, label %for.inc9

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 5, %for.body4.preheader ]
  %3 = trunc i64 %indvars.iv to i32
  %rem = srem i32 %3, %m2
  %add5 = add nsw i32 %add, %rem
  %mul = mul nsw i32 %add5, %3
  %add6 = add nsw i32 %mul, %2
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %indvars.iv32, i64 %indvars.iv
  store i32 %add6, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp3 = icmp slt i64 %indvars.iv, %1
  br i1 %cmp3, label %for.body4, label %for.inc9.loopexit

for.inc9.loopexit:                                ; preds = %for.body4
  %add5.lcssa = phi i32 [ %add5, %for.body4 ]
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.body
  %n.1.lcssa = phi i32 [ %n.031, %for.body ], [ %add5.lcssa, %for.inc9.loopexit ]
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond = icmp eq i64 %indvars.iv.next33, %wide.trip.count
  br i1 %exitcond, label %for.end11.loopexit, label %for.body

for.end11.loopexit:                               ; preds = %for.inc9
  br label %for.end11

for.end11:                                        ; preds = %for.end11.loopexit, %entry
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang d3502e2096448b81a99df35eb5758566d27dd199) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ec5c2ba25bc3213af1e1903881ec25e4e07e6b92)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_i", !4, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
