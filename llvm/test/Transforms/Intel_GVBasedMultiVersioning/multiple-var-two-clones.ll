; RUN: opt -S -passes=gvbased-multiversioning -gvbased-multiversion-min-num-branches=3 < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global1 = internal unnamed_addr global i1 false, align 1
@global2 = internal unnamed_addr global i1 false, align 1
@global3 = internal unnamed_addr global i1 false, align 1

declare void @unknown_side_effect() local_unnamed_addr #0
declare void @unknown_side_effect2() local_unnamed_addr #0

; void multiversion_multi_vars_2_clones(int *num, int *src, int n) {
;   if (global1) {
;     if (global2)
;       unknown_side_effect();
;   } else {
;     if (!global3)
;       unknown_side_effect2();
;   }
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
;   if (global1) {
;     if (global2)
;       unknown_side_effect();
;   } else {
;     if (!global3)
;       unknown_side_effect2();
;   }
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
;   if (global1) {
;     if (global2) {
;       unknown_side_effect();
;       return;
;     }
;   } else {
;     if (!global3) {
;       unknown_side_effect2();
;       return;
;     }
;   }
;   #pragma nounroll
;   for (int i = 0; i < n; i++)
;     num[i] += src[i];
; }
;
; InvariantSets: [[(@global1, true), (@global2, false)], [(@global1, false), (@global3, true)]]
; Also checks determinism of multiversioning analysis because the number of uses of global1, global2, global3 are the same. Invariants should always be created from global1.
define void @multiversion_multi_vars_2_clones(ptr nocapture noundef %num, ptr nocapture noundef readonly %src, i32 noundef %n) #0 {
; CHECK-LABEL: define void @multiversion_multi_vars_2_clones
; CHECK-NEXT:  mv.global.loads:
; CHECK-NEXT:    [[MV_LOAD_GLOBAL1:%.*]] = load i1, ptr @global1, align 1
; CHECK-NEXT:    [[MV_LOAD_GLOBAL2:%.*]] = load i1, ptr @global2, align 1
; CHECK-NEXT:    [[MV_LOAD_GLOBAL3:%.*]] = load i1, ptr @global3, align 1
; CHECK-NEXT:    [[TMP0:%.*]] = xor i1 [[MV_LOAD_GLOBAL1]], true
; CHECK-NEXT:    [[TMP1:%.*]] = and i1 [[TMP0]], [[MV_LOAD_GLOBAL3]]
; CHECK-NEXT:    br i1 [[TMP1]], label [[ENTRY_CLONE1:%.*]], label %mv.cond
; CHECK:       mv.cond:
; CHECK-NEXT:    [[TMP2:%.*]] = xor i1 [[MV_LOAD_GLOBAL2]], true
; CHECK-NEXT:    [[TMP3:%.*]] = and i1 [[MV_LOAD_GLOBAL1]], [[TMP2]]
; CHECK-NEXT:    br i1 [[TMP3]], label [[ENTRY_CLONE:%.*]], label [[ENTRY:%.*]]
entry:
  %.b71 = load i1, ptr @global1, align 4
  br i1 %.b71, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %.b74 = load i1, ptr @global2, align 4
  br i1 %.b74, label %if.then2, label %if.end6

if.then2:                                         ; preds = %if.then
  tail call void @unknown_side_effect()
  br label %if.end6

if.else:                                          ; preds = %entry
  %.b77 = load i1, ptr @global3, align 4
  br i1 %.b77, label %if.end6, label %if.then4

if.then4:                                         ; preds = %if.else
  tail call void @unknown_side_effect2()
  br label %if.end6

if.end6:                                          ; preds = %if.else, %if.then4, %if.then, %if.then2
  %cmp78 = icmp sgt i32 %n, 0
  br i1 %cmp78, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %if.end6
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %if.end6
  %.b70 = load i1, ptr @global1, align 4
  br i1 %.b70, label %if.then10, label %if.else14

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %src, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %arrayidx8 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx8, align 4, !tbaa !3
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx8, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body

if.then10:                                        ; preds = %for.cond.cleanup
  %.b73 = load i1, ptr @global2, align 4
  br i1 %.b73, label %if.then12, label %if.end18

if.then12:                                        ; preds = %if.then10
  tail call void @unknown_side_effect()
  br label %if.end18

if.else14:                                        ; preds = %for.cond.cleanup
  %.b76 = load i1, ptr @global3, align 4
  br i1 %.b76, label %if.end18, label %if.then16

if.then16:                                        ; preds = %if.else14
  tail call void @unknown_side_effect2()
  br label %if.end18

if.end18:                                         ; preds = %if.else14, %if.then16, %if.then10, %if.then12
  br i1 %cmp78, label %for.body23.preheader, label %for.cond.cleanup22

for.body23.preheader:                             ; preds = %if.end18
  %wide.trip.count86 = zext i32 %n to i64
  br label %for.body23

for.cond.cleanup22:                               ; preds = %for.body23, %if.end18
  %.b = load i1, ptr @global1, align 4
  br i1 %.b, label %if.then33, label %if.else37

for.body23:                                       ; preds = %for.body23.preheader, %for.body23
  %indvars.iv84 = phi i64 [ 0, %for.body23.preheader ], [ %indvars.iv.next85, %for.body23 ]
  %arrayidx25 = getelementptr inbounds i32, ptr %src, i64 %indvars.iv84
  %2 = load i32, ptr %arrayidx25, align 4, !tbaa !3
  %arrayidx27 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv84
  %3 = load i32, ptr %arrayidx27, align 4, !tbaa !3
  %add28 = add nsw i32 %3, %2
  store i32 %add28, ptr %arrayidx27, align 4, !tbaa !3
  %indvars.iv.next85 = add nuw nsw i64 %indvars.iv84, 1
  %exitcond87.not = icmp eq i64 %indvars.iv.next85, %wide.trip.count86
  br i1 %exitcond87.not, label %for.cond.cleanup22, label %for.body23

if.then33:                                        ; preds = %for.cond.cleanup22
  %.b72 = load i1, ptr @global2, align 4
  br i1 %.b72, label %if.then35, label %if.end41

if.then35:                                        ; preds = %if.then33
  tail call void @unknown_side_effect()
  br label %for.end54

if.else37:                                        ; preds = %for.cond.cleanup22
  %.b75 = load i1, ptr @global3, align 4
  br i1 %.b75, label %if.end41, label %if.then39

if.then39:                                        ; preds = %if.else37
  tail call void @unknown_side_effect2()
  br label %for.end54

if.end41:                                         ; preds = %if.else37, %if.then33
  br i1 %cmp78, label %for.body46.preheader, label %for.end54

for.body46.preheader:                             ; preds = %if.end41
  %wide.trip.count90 = zext i32 %n to i64
  br label %for.body46

for.body46:                                       ; preds = %for.body46.preheader, %for.body46
  %indvars.iv88 = phi i64 [ 0, %for.body46.preheader ], [ %indvars.iv.next89, %for.body46 ]
  %arrayidx48 = getelementptr inbounds i32, ptr %src, i64 %indvars.iv88
  %4 = load i32, ptr %arrayidx48, align 4, !tbaa !3
  %arrayidx50 = getelementptr inbounds i32, ptr %num, i64 %indvars.iv88
  %5 = load i32, ptr %arrayidx50, align 4, !tbaa !3
  %add51 = add nsw i32 %5, %4
  store i32 %add51, ptr %arrayidx50, align 4, !tbaa !3
  %indvars.iv.next89 = add nuw nsw i64 %indvars.iv88, 1
  %exitcond91.not = icmp eq i64 %indvars.iv.next89, %wide.trip.count90
  br i1 %exitcond91.not, label %for.end54, label %for.body46

for.end54:                                        ; preds = %for.body46, %if.end41, %if.then35, %if.then39
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
