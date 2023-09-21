; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = internal unnamed_addr global i1 false, align 1

declare void @unknown_side_effect() local_unnamed_addr #0

; void multiversion_1_var_2_clones(int * restrict num, int * restrict src, int n) {
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
;   if (global1) {
;     num[0] += 1;
;   }
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
;   if (global1) {
;     num[1] += 1;
;   }
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
;   if (global1) {
;     num[2] += 1;
;   }
; }
;
; InvariantSets: [[(@global1, true)], [(@global1, false)]]
; Check that there are only 2 copies of code, the original code is reused for one of the copies.
define void @multiversion_1_var_2_clones(ptr noalias nocapture noundef %num, ptr noalias nocapture noundef readonly %src, i32 noundef %n) #0 {
; CHECK-LABEL: define void @multiversion_1_var_2_clones
; CHECK-NEXT:  mv.global.loads:
; CHECK-NEXT:    [[MV_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    br i1 [[MV_LOAD_GLOBAL1]], label %entry.clone, label %entry
; CHECK:       entry:
; CHECK-NEXT:    [[CMP61:%.*]] = icmp sgt i32 %n, 0
; CHECK-NEXT:    br i1 [[CMP61]], label %for.body.preheader, label %if.end40
; CHECK:       for.body.preheader:
; CHECK-NEXT:    [[WIDE_TRIP_COUNT:%.*]] = zext i32 %n to i64
; CHECK-NEXT:    br label %for.body
; This BB is different in the two clones because we fold the invariant of @global1.
; CHECK:       if.end:
; CHECK-NEXT:    br i1 [[CMP61]], label [[FOR_BODY9_PREHEADER:%.*]], label [[FOR_COND_CLEANUP8:%.*]]
; CHECK:       if.end40:
; CHECK-NEXT:    ret void
;
; CHECK:       entry.clone:
; CHECK-NEXT:    [[CMP61_CLONE:%.*]] = icmp sgt i32 %n, 0
; CHECK-NEXT:    br i1 [[CMP61_CLONE]], label %for.body.preheader.clone, label %if.then19.clone.critedge
; CHECK:       for.body.preheader.clone:
; CHECK-NEXT:    [[WIDE_TRIP_COUNT_CLONE:%.*]] = zext i32 %n to i64
; CHECK-NEXT:    br label %for.body.clone
; CHECK:       if.then19.clone.critedge:
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr %num, align 4
; CHECK-NEXT:    [[ADD4_CLONE:%.*]] = add nsw i32 [[TMP6]], 1
; CHECK-NEXT:    store i32 [[ADD4_CLONE]], ptr %num, align 4
; CHECK-NEXT:    br label %if.then19.clone
entry:
  %cmp61 = icmp sgt i32 %n, 0
  br i1 %cmp61, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %.b60 = load i1, ptr @global1, align 4
  br i1 %.b60, label %if.then, label %if.end

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %src, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx2, align 4, !tbaa !3
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx2, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

if.then:                                          ; preds = %for.cond.cleanup
  %2 = load i32, ptr %num, align 4, !tbaa !3
  %add4 = add nsw i32 %2, 1
  store i32 %add4, ptr %num, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.cond.cleanup
  br i1 %cmp61, label %for.body9.preheader, label %for.cond.cleanup8

for.body9.preheader:                              ; preds = %if.end
  %wide.trip.count69 = zext i32 %n to i64
  br label %for.body9

for.cond.cleanup8:                                ; preds = %for.body9, %if.end
  br i1 %.b60, label %if.then19, label %if.end22

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv67 = phi i64 [ 0, %for.body9.preheader ], [ %indvars.iv.next68, %for.body9 ]
  %arrayidx11 = getelementptr inbounds i32, ptr %src, i64 %indvars.iv67
  %3 = load i32, ptr %arrayidx11, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv67
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !3
  %add14 = add nsw i32 %4, %3
  store i32 %add14, ptr %arrayidx13, align 4, !tbaa !3
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond70.not = icmp eq i64 %indvars.iv.next68, %wide.trip.count69
  br i1 %exitcond70.not, label %for.cond.cleanup8, label %for.body9

if.then19:                                        ; preds = %for.cond.cleanup8
  %arrayidx20 = getelementptr inbounds i32, ptr %num, i64 1
  %5 = load i32, ptr %arrayidx20, align 4, !tbaa !3
  %add21 = add nsw i32 %5, 1
  store i32 %add21, ptr %arrayidx20, align 4, !tbaa !3
  br label %if.end22

if.end22:                                         ; preds = %if.then19, %for.cond.cleanup8
  br i1 %cmp61, label %for.body27.preheader, label %for.cond.cleanup26

for.body27.preheader:                             ; preds = %if.end22
  %wide.trip.count73 = zext i32 %n to i64
  br label %for.body27

for.cond.cleanup26:                               ; preds = %for.body27, %if.end22
  br i1 %.b60, label %if.then37, label %if.end40

for.body27:                                       ; preds = %for.body27.preheader, %for.body27
  %indvars.iv71 = phi i64 [ 0, %for.body27.preheader ], [ %indvars.iv.next72, %for.body27 ]
  %arrayidx29 = getelementptr inbounds i32, ptr %src, i64 %indvars.iv71
  %6 = load i32, ptr %arrayidx29, align 4, !tbaa !3
  %arrayidx31 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv71
  %7 = load i32, ptr %arrayidx31, align 4, !tbaa !3
  %add32 = add nsw i32 %7, %6
  store i32 %add32, ptr %arrayidx31, align 4, !tbaa !3
  %indvars.iv.next72 = add nuw nsw i64 %indvars.iv71, 1
  %exitcond74.not = icmp eq i64 %indvars.iv.next72, %wide.trip.count73
  br i1 %exitcond74.not, label %for.cond.cleanup26, label %for.body27

if.then37:                                        ; preds = %for.cond.cleanup26
  %arrayidx38 = getelementptr inbounds i32, ptr %num, i64 2
  %8 = load i32, ptr %arrayidx38, align 4, !tbaa !3
  %add39 = add nsw i32 %8, 1
  store i32 %add39, ptr %arrayidx38, align 4, !tbaa !3
  br label %if.end40

if.end40:                                         ; preds = %if.then37, %for.cond.cleanup26
  ret void
}

attributes #0 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
