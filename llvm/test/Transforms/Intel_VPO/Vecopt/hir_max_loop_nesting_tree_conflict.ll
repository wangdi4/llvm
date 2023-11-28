;
; RUN: opt -S -mattr=+avx512vl,+avx512cd -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -enable-vconflict-idiom -vplan-force-vf=4 -disable-output -debug-only=VPlanDriver < %s 2>&1 | FileCheck %s
;
; LIT test to check that we do not exceed HIR maximum loop nesting level when vectorizing
; a loop with tree conflict lowering which introduces a new loop there by potentially
; exceeding maximum allowed loop nesting level.
;
; Incoming scalar HIR:
; Function: nesting9
;
;  + DO i9 = 0, 99, 1   <DO_LOOP>
;  |   %8 = (%c)[i9];
;  |   %9 = (%a)[i9];
;  |   %10 = (%7)[%9];
;  |   (%7)[%9] = %8 + %10;
;  + END LOOP
;
; Function: nesting8
;
;  + DO i8 = 0, 99, 1   <DO_LOOP>
;  |   %7 = (%c)[i8];
;  |   %8 = (%a)[i8];
;  |   %9 = (%6)[%8];
;  |   (%6)[%8] = %7 + %9;
;  + END LOOP
;
;
; CHECK:        VD: Not vectorizing: No VPlans constructed.
; CHECK:        Function: nesting9
; CHECK:        DO i9 = 0, 99, 1   <DO_LOOP>
;
; CHECK:        Function: nesting8
; CHECK:        DO i8 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @nesting9(ptr noalias nocapture noundef readonly %lp0, ptr noalias nocapture noundef readonly %a, ptr noalias nocapture noundef readonly %c) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end55
  %l1.0103 = phi i64 [ 0, %entry ], [ %inc57, %for.end55 ]
  %arrayidx = getelementptr inbounds ptr, ptr %lp0, i64 %l1.0103
  %0 = load ptr, ptr %arrayidx, align 8
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.end52
  %l2.0102 = phi i64 [ 0, %for.body ], [ %inc54, %for.end52 ]
  %arrayidx4 = getelementptr inbounds ptr, ptr %0, i64 %l2.0102
  %1 = load ptr, ptr %arrayidx4, align 8
  br label %for.body7

for.body7:                                        ; preds = %for.body3, %for.end49
  %l3.0101 = phi i64 [ 0, %for.body3 ], [ %inc51, %for.end49 ]
  %arrayidx8 = getelementptr inbounds ptr, ptr %1, i64 %l3.0101
  %2 = load ptr, ptr %arrayidx8, align 8
  br label %for.body11

for.body11:                                       ; preds = %for.body7, %for.end46
  %l4.0100 = phi i64 [ 0, %for.body7 ], [ %inc48, %for.end46 ]
  %arrayidx12 = getelementptr inbounds ptr, ptr %2, i64 %l4.0100
  %3 = load ptr, ptr %arrayidx12, align 8
  br label %for.body15

for.body15:                                       ; preds = %for.body11, %for.end43
  %l5.099 = phi i64 [ 0, %for.body11 ], [ %inc45, %for.end43 ]
  %arrayidx16 = getelementptr inbounds ptr, ptr %3, i64 %l5.099
  %4 = load ptr, ptr %arrayidx16, align 8
  br label %for.body19

for.body19:                                       ; preds = %for.body15, %for.end40
  %l6.098 = phi i64 [ 0, %for.body15 ], [ %inc42, %for.end40 ]
  %arrayidx20 = getelementptr inbounds ptr, ptr %4, i64 %l6.098
  %5 = load ptr, ptr %arrayidx20, align 8
  br label %for.body23

for.body23:                                       ; preds = %for.body19, %for.end37
  %l7.097 = phi i64 [ 0, %for.body19 ], [ %inc39, %for.end37 ]
  %arrayidx24 = getelementptr inbounds ptr, ptr %5, i64 %l7.097
  %6 = load ptr, ptr %arrayidx24, align 8
  br label %for.body27

for.body27:                                       ; preds = %for.body23, %for.end
  %l8.096 = phi i64 [ 0, %for.body23 ], [ %inc36, %for.end ]
  %arrayidx28 = getelementptr inbounds ptr, ptr %6, i64 %l8.096
  %7 = load ptr, ptr %arrayidx28, align 8
  br label %for.body31

for.body31:                                       ; preds = %for.body27, %for.body31
  %l9.095 = phi i64 [ 0, %for.body27 ], [ %inc, %for.body31 ]
  %arrayidx32 = getelementptr inbounds i64, ptr %c, i64 %l9.095
  %8 = load i64, ptr %arrayidx32, align 8
  %arrayidx33 = getelementptr inbounds i64, ptr %a, i64 %l9.095
  %9 = load i64, ptr %arrayidx33, align 8
  %arrayidx34 = getelementptr inbounds i64, ptr %7, i64 %9
  %10 = load i64, ptr %arrayidx34, align 8
  %add = add nsw i64 %10, %8
  store i64 %add, ptr %arrayidx34, align 8
  %inc = add nuw nsw i64 %l9.095, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body31

for.end:                                          ; preds = %for.body31
  %inc36 = add nuw nsw i64 %l8.096, 1
  %exitcond104.not = icmp eq i64 %inc36, 100
  br i1 %exitcond104.not, label %for.end37, label %for.body27

for.end37:                                        ; preds = %for.end
  %inc39 = add nuw nsw i64 %l7.097, 1
  %exitcond105.not = icmp eq i64 %inc39, 100
  br i1 %exitcond105.not, label %for.end40, label %for.body23

for.end40:                                        ; preds = %for.end37
  %inc42 = add nuw nsw i64 %l6.098, 1
  %exitcond106.not = icmp eq i64 %inc42, 100
  br i1 %exitcond106.not, label %for.end43, label %for.body19

for.end43:                                        ; preds = %for.end40
  %inc45 = add nuw nsw i64 %l5.099, 1
  %exitcond107.not = icmp eq i64 %inc45, 100
  br i1 %exitcond107.not, label %for.end46, label %for.body15

for.end46:                                        ; preds = %for.end43
  %inc48 = add nuw nsw i64 %l4.0100, 1
  %exitcond108.not = icmp eq i64 %inc48, 100
  br i1 %exitcond108.not, label %for.end49, label %for.body11

for.end49:                                        ; preds = %for.end46
  %inc51 = add nuw nsw i64 %l3.0101, 1
  %exitcond109.not = icmp eq i64 %inc51, 100
  br i1 %exitcond109.not, label %for.end52, label %for.body7

for.end52:                                        ; preds = %for.end49
  %inc54 = add nuw nsw i64 %l2.0102, 1
  %exitcond110.not = icmp eq i64 %inc54, 100
  br i1 %exitcond110.not, label %for.end55, label %for.body3

for.end55:                                        ; preds = %for.end52
  %inc57 = add nuw nsw i64 %l1.0103, 1
  %exitcond111.not = icmp eq i64 %inc57, 100
  br i1 %exitcond111.not, label %for.end58, label %for.body

for.end58:                                        ; preds = %for.end55
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @nesting8(ptr noalias nocapture noundef readonly %lp0, ptr noalias nocapture noundef readonly %a, ptr noalias nocapture noundef readonly %c) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.end48
  %l1.091 = phi i64 [ 0, %entry ], [ %inc50, %for.end48 ]
  %arrayidx = getelementptr inbounds ptr, ptr %lp0, i64 %l1.091
  %0 = load ptr, ptr %arrayidx, align 8
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.end45
  %l2.090 = phi i64 [ 0, %for.body ], [ %inc47, %for.end45 ]
  %arrayidx4 = getelementptr inbounds ptr, ptr %0, i64 %l2.090
  %1 = load ptr, ptr %arrayidx4, align 8
  br label %for.body7

for.body7:                                        ; preds = %for.body3, %for.end42
  %l3.089 = phi i64 [ 0, %for.body3 ], [ %inc44, %for.end42 ]
  %arrayidx8 = getelementptr inbounds ptr, ptr %1, i64 %l3.089
  %2 = load ptr, ptr %arrayidx8, align 8
  br label %for.body11

for.body11:                                       ; preds = %for.body7, %for.end39
  %l4.088 = phi i64 [ 0, %for.body7 ], [ %inc41, %for.end39 ]
  %arrayidx12 = getelementptr inbounds ptr, ptr %2, i64 %l4.088
  %3 = load ptr, ptr %arrayidx12, align 8
  br label %for.body15

for.body15:                                       ; preds = %for.body11, %for.end36
  %l5.087 = phi i64 [ 0, %for.body11 ], [ %inc38, %for.end36 ]
  %arrayidx16 = getelementptr inbounds ptr, ptr %3, i64 %l5.087
  %4 = load ptr, ptr %arrayidx16, align 8
  br label %for.body19

for.body19:                                       ; preds = %for.body15, %for.end33
  %l6.086 = phi i64 [ 0, %for.body15 ], [ %inc35, %for.end33 ]
  %arrayidx20 = getelementptr inbounds ptr, ptr %4, i64 %l6.086
  %5 = load ptr, ptr %arrayidx20, align 8
  br label %for.body23

for.body23:                                       ; preds = %for.body19, %for.end
  %l7.085 = phi i64 [ 0, %for.body19 ], [ %inc32, %for.end ]
  %arrayidx24 = getelementptr inbounds ptr, ptr %5, i64 %l7.085
  %6 = load ptr, ptr %arrayidx24, align 8
  br label %for.body27

for.body27:                                       ; preds = %for.body23, %for.body27
  %l8.084 = phi i64 [ 0, %for.body23 ], [ %inc, %for.body27 ]
  %arrayidx28 = getelementptr inbounds i64, ptr %c, i64 %l8.084
  %7 = load i64, ptr %arrayidx28, align 8
  %arrayidx29 = getelementptr inbounds i64, ptr %a, i64 %l8.084
  %8 = load i64, ptr %arrayidx29, align 8
  %arrayidx30 = getelementptr inbounds i64, ptr %6, i64 %8
  %9 = load i64, ptr %arrayidx30, align 8
  %add = add nsw i64 %9, %7
  store i64 %add, ptr %arrayidx30, align 8
  %inc = add nuw nsw i64 %l8.084, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body27

for.end:                                          ; preds = %for.body27
  %inc32 = add nuw nsw i64 %l7.085, 1
  %exitcond92.not = icmp eq i64 %inc32, 100
  br i1 %exitcond92.not, label %for.end33, label %for.body23

for.end33:                                        ; preds = %for.end
  %inc35 = add nuw nsw i64 %l6.086, 1
  %exitcond93.not = icmp eq i64 %inc35, 100
  br i1 %exitcond93.not, label %for.end36, label %for.body19

for.end36:                                        ; preds = %for.end33
  %inc38 = add nuw nsw i64 %l5.087, 1
  %exitcond94.not = icmp eq i64 %inc38, 100
  br i1 %exitcond94.not, label %for.end39, label %for.body15

for.end39:                                        ; preds = %for.end36
  %inc41 = add nuw nsw i64 %l4.088, 1
  %exitcond95.not = icmp eq i64 %inc41, 100
  br i1 %exitcond95.not, label %for.end42, label %for.body11

for.end42:                                        ; preds = %for.end39
  %inc44 = add nuw nsw i64 %l3.089, 1
  %exitcond96.not = icmp eq i64 %inc44, 100
  br i1 %exitcond96.not, label %for.end45, label %for.body7

for.end45:                                        ; preds = %for.end42
  %inc47 = add nuw nsw i64 %l2.090, 1
  %exitcond97.not = icmp eq i64 %inc47, 100
  br i1 %exitcond97.not, label %for.end48, label %for.body3

for.end48:                                        ; preds = %for.end45
  %inc50 = add nuw nsw i64 %l1.091, 1
  %exitcond98.not = icmp eq i64 %inc50, 100
  br i1 %exitcond98.not, label %for.end51, label %for.body

for.end51:                                        ; preds = %for.end48
  ret void
}
