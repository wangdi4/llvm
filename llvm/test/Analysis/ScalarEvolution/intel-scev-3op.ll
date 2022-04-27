; Loop limit is a 3-operand SCEV:
; (103 + (zext i1 %tobool to i32) + (-1 * (zext i16 (86 smax %a) to i32))<nsw>)<nsw>
; SCEV isImpliedViaOperations is assuming 2-operand SCEVs and leaving out the
; very important 3rd term, resulting in "always-positive".

; RUN: opt -indvars -S < %s | FileCheck %s
; RUN: opt -passes=indvars -S < %s | FileCheck %s
; CHECK-NOT: br{{.*}}true
; CHECK-NOT: br{{.*}}false

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@arr_461 = dso_local local_unnamed_addr global [18 x i8] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @_Z4testsy(i16 signext %a, i64 %g) local_unnamed_addr #1 {
entry:
  %tobool = icmp eq i64 %g, 0
  %conv = zext i1 %tobool to i32
  %0 = icmp sgt i16 %a, 86
  %1 = select i1 %0, i16 %a, i16 86
  %conv1 = zext i16 %1 to i32
  %sub = sub nsw i32 103, %conv1
  %add = add nsw i32 %sub, %conv
  %cmp6 = icmp sgt i32 %add, 0
  br i1 %cmp6, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %f.07 = phi i32 [ %add2, %for.body ], [ 0, %for.body.preheader ]
  %idxprom = zext i32 %f.07 to i64
  %arrayidx = getelementptr inbounds [18 x i8], [18 x i8]* @arr_461, i64 0, i64 %idxprom, !intel-tbaa !2
  store i8 1, i8* %arrayidx, align 4, !tbaa !2
  %add2 = add nuw nsw i32 %f.07, 4
  %cmp = icmp slt i32 %add2, %add
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA18_b", !4, i64 0}
!4 = !{!"bool", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"short", !5, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"long long", !5, i64 0}
!11 = !{i8 0, i8 2}
