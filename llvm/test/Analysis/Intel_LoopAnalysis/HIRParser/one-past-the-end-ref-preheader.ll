; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we are able to parse this test case successfully. Framework was failing when trying to restructure one-past-the-end lval ref in i2 loop preheader.

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][%t * i1 + %t + 10];
; CHECK: |
; CHECK: |      (@A)[0][%t * i1 + %t + 10] = i1 + %0;
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   |   %1 = (@A)[0][i2];
; CHECK: |   |   (@A)[0][i2] = i2 + %1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n, i64 %t) local_unnamed_addr #0 {
entry:
  %cmp216 = icmp sgt i32 %n, 0
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc5, %entry
  %p.019 = phi ptr [ getelementptr inbounds ([10 x i32], ptr @A, i64 1, i64 0), %entry ], [ %incdec.ptr, %for.inc5 ]
  %i.018 = phi i32 [ 0, %entry ], [ %inc6, %for.inc5 ]
  %incdec.ptr = getelementptr inbounds i32, ptr %p.019, i64 %t
  %0 = load i32, ptr %incdec.ptr, align 4, !tbaa !2
  %add = add nsw i32 %0, %i.018
  br i1 %cmp216, label %for.body3.preheader, label %for.inc5

for.body3.preheader:                              ; preds = %for.body
  store i32 %add, ptr %incdec.ptr, align 4, !tbaa !2
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %2 = trunc i64 %indvars.iv to i32
  %add4 = add nsw i32 %1, %2
  store i32 %add4, ptr %arrayidx, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:                                ; preds = %for.body3
  br label %for.inc5

for.inc5:                                         ; preds = %for.inc5.loopexit, %for.body
  %inc6 = add nuw nsw i32 %i.018, 1
  %exitcond20 = icmp eq i32 %inc6, 10
  br i1 %exitcond20, label %for.end7, label %for.body

for.end7:                                         ; preds = %for.inc5
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5f4f662867240aabc27bc9491b73d220049214d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1253b775d58f338f3e4a88a40c6f379413bf8466)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA10_i", !3, i64 0}
