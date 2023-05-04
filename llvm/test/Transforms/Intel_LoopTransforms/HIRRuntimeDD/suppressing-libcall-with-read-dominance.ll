; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed  -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-runtime-dd-read-dominance-threshold=5 < %s 2>&1 | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed  -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-runtime-dd-rtl-threshold=1  -hir-runtime-dd-read-dominance-threshold=2 < %s 2>&1 | FileCheck %s --check-prefix="NOLIBCALL"
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -hir-details -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s --check-prefix="NOLIBCALL1"
; ModuleID = 'fast_test.c'
source_filename = "fast_test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check runtime dd multiversioning without library call.

; In this simple example loop, there are 4 vals in total, and 3 of them are
; read-only vals. Test size is 3.
; By default it would inlined regular RTDD because test size is only 3 which
; is small than threshold 16. No matter if adding
; -hir-runtime-dd-read-dominance-threshold=5 or not.
; Unless with -hir-runtime-dd-rtl-threshold=1 or 2, it would call __intel_rtdd_indep.
; But if adding hir-runtime-dd-read-dominance-threshold=2, which means it is
; read dominant if number of read-only vals is more than 2 times of write vals.
; Then it would inline regular RTDD instead of calling library RTDD.

;         BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64((1 + %M)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   %2 = (%A)[i1];
;               |   (%D)[i1 + 1] = 2 * %2;
;               |   %3 = (%B)[i1 + 1];
;               |   %4 = (%C)[i1 + 1];
;               |   (%D)[i1 + 1] = smax((%3 + %4), (2 * %2));
;               + END LOOP
;         END REGION

;CHECK:               Function: foo
;CHECK-NOT:           %call = @__intel_rtdd_indep;
;CHECK:               %mv.test5 = &((%D)[zext.i32.i64(%M)]) >=u &((%C)[1]);
;CHECK:               %mv.test6 = &((%C)[zext.i32.i64(%M)]) >=u &((%D)[1]);
;CHECK:               %mv.and7 = %mv.test5  &  %mv.test6;
;CHECK:               if (%mv.and == 0 & %mv.and4 == 0 & %mv.and7 == 0)

;NOLIBCALL:           Function: foo
;NOLIBCALL-NOT:       %call = @__intel_rtdd_indep;
;NOLIBCALL:           %mv.test5 = &((%D)[zext.i32.i64(%M)]) >=u &((%C)[1]);
;NOLIBCALL:           %mv.test6 = &((%C)[zext.i32.i64(%M)]) >=u &((%D)[1]);
;NOLIBCALL:           %mv.and7 = %mv.test5  &  %mv.test6;
;NOLIBCALL:           if (%mv.and == 0 & %mv.and4 == 0 & %mv.and7 == 0)

define dso_local void @foo(ptr nocapture noundef readonly %A, ptr nocapture noundef readonly %B, ptr nocapture noundef readonly %C, ptr nocapture noundef writeonly %D, i32 noundef %M) local_unnamed_addr #0 {
entry:
  %cmp.not21 = icmp slt i32 %M, 1
  br i1 %cmp.not21, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %0 = add nuw nsw i32 %M, 1
  %wide.trip.count = zext i32 %0 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 1, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %1
  %2 = load i32, ptr %arrayidx, align 4
  %mul = shl nsw i32 %2, 1
  %arrayidx2 = getelementptr inbounds i32, ptr %D, i64 %indvars.iv
  store i32 %mul, ptr %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx6, align 4
  %add = add nsw i32 %4, %3
  %spec.store.select = tail call i32 @llvm.smax.i32(i32 %add, i32 %mul)
  store i32 %spec.store.select, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

declare i32 @llvm.smax.i32(i32, i32)


; It is previously lib call but now regular inlined for runtime DD checking.
; In this loop, there are 16 vals in total, and 13 of them are
; read-only vals. If there is a overlap between the 13 read-only vals,
; RTDD library call would not pass and it might result in further
; optimization opportunities loss.
; But actually no matter read-only vals have overlap or not, the case could
; run to optimized path. So there is a heuristic to supress the runtime DD
; library call if only load Rvals are much more than Lvals.
; To make it call library RTDD, we could try to add -hir-runtime-dd-read-dominance-threshold >= 5

;          BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64((1 + %n1)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>  <LEGAL_MAX_TC = 2147483648>
;               |   %3 = (%mpp)[i1 + -1];
;               |   %4 = (%tpmm)[i1 + -1];
;               |   (%mc)[i1] = %3 + %4;
;               |   %5 = (%ip)[i1 + -1];
;               |   %6 = (%tpim)[i1 + -1];
;               |   (%mc)[i1] = smax((%5 + %6), (%3 + %4));
;               |   %7 = (%dpp)[i1 + -1];
;               |   %8 = (%tpdm)[i1 + -1];
;               |   (%mc)[i1] = smax((%7 + %8), (%5 + %6), (%3 + %4));
;               |   %9 = (%bp)[i1];
;               |   (%mc)[i1] = smax((%xmb + %9), (%7 + %8), (%5 + %6), (%3 + %4));
;               |   %10 = (%ms)[i1];
;               |   (%mc)[i1] = smax(-987654321, (smax((%xmb + %9), (%7 + %8), (%5 + %6), (%3 + %4)) + %10));
;               |   %11 = (%dc)[i1 + -1];
;               |   %12 = (%tpdd)[i1 + -1];
;               |   (%dc)[i1] = %11 + %12;
;               |   %13 = (%mc)[i1 + -1];
;               |   %14 = (%tpmd)[i1 + -1];
;               |   (%dc)[i1] = smax(-987654321, (%13 + %14), (%11 + %12));
;               |   if (i1 <u %n1)
;               |   {
;               |      %15 = (%mpp)[i1];
;               |      %16 = (%tpmi)[i1];
;               |      (%ic)[i1] = %15 + %16;
;               |      %17 = (%ip)[i1];
;               |      %18 = (%tpii)[i1];
;               |      (%ic)[i1] = smax((%17 + %18), (%15 + %16));
;               |      %19 = (%is)[i1];
;               |      (%ic)[i1] = smax(-987654321, (smax((%17 + %18), (%15 + %16)) + %19));
;               |   }
;               + END LOOP
;          END REGION

; CHECK:            Function: P7Viterbi
; CHECK:            (%dd)[0][15].0 = &((%is)[0]);
; CHECK:            (%dd)[0][15].1 = &((%is)[zext.i32.i64(%n1)]);
; CHECK:            %call = @__intel_rtdd_indep(&((i8*)(%dd)[0]),  16);
; CHECK:            if (%call == 0)  <MVTag: 89>

; NOLIBCALL1:       Function: P7Viterbi
; NOLIBCALL1-NOT:   __intel_rtdd_indep
; NOLIBCALL1:       %mv.test122 = &((%ic)[zext.i32.i64(%n1)]) >=u &((%is)[0]);
; NOLIBCALL1:       %mv.test123 = &((%is)[zext.i32.i64(%n1)]) >=u &((%ic)[0]);
; NOLIBCALL1:       %mv.and124 = %mv.test122  &  %mv.test123;
; NOLIBCALL1:       if (%mv.and == 0 & %mv.and4 == 0

define dso_local float @P7Viterbi(ptr nocapture noundef writeonly %dsq, i32 noundef %L, ptr nocapture noundef readonly %ms, ptr nocapture noundef readonly %is, i32 noundef %n1, i32 noundef %n2, i32 noundef %n3, ptr nocapture noundef %mc, ptr nocapture noundef %dc, ptr nocapture noundef %ic, ptr nocapture noundef readonly %tpmm, ptr nocapture noundef readonly %tpmi, ptr nocapture noundef readonly %tpmd, ptr nocapture noundef readonly %tpim, ptr nocapture noundef readonly %tpii, ptr nocapture noundef readonly %tpdm, ptr nocapture noundef readonly %tpdd, ptr nocapture noundef readonly %bp, ptr nocapture noundef readonly %mpp, ptr nocapture noundef readnone %mpc, ptr nocapture noundef readonly %ip, ptr nocapture noundef readonly %dpp, i32 noundef %xmb) local_unnamed_addr #0 {
entry:
  %cmp.not211 = icmp slt i32 %n1, 0
  br i1 %cmp.not211, label %entry.for.end_crit_edge, label %for.body.preheader

entry.for.end_crit_edge:                          ; preds = %entry
  %.pre = sext i32 %n1 to i64
  br label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = zext i32 %n1 to i64
  %1 = add nuw nsw i32 %n1, 1
  %wide.trip.count = zext i32 %1 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, ptr %mpp, i64 %2
  %3 = load i32, ptr %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds i32, ptr %tpmm, i64 %2
  %4 = load i32, ptr %arrayidx3, align 4
  %add = add nsw i32 %4, %3
  %arrayidx5 = getelementptr inbounds i32, ptr %mc, i64 %indvars.iv
  store i32 %add, ptr %arrayidx5, align 4
  %arrayidx8 = getelementptr inbounds i32, ptr %ip, i64 %2
  %5 = load i32, ptr %arrayidx8, align 4
  %arrayidx11 = getelementptr inbounds i32, ptr %tpim, i64 %2
  %6 = load i32, ptr %arrayidx11, align 4
  %add12 = add nsw i32 %6, %5
  %cmp15 = icmp sgt i32 %add12, %add
  %spec.store.select = select i1 %cmp15, i32 %add12, i32 %add
  store i32 %spec.store.select, ptr %arrayidx5, align 4
  %arrayidx20 = getelementptr inbounds i32, ptr %dpp, i64 %2
  %7 = load i32, ptr %arrayidx20, align 4
  %arrayidx23 = getelementptr inbounds i32, ptr %tpdm, i64 %2
  %8 = load i32, ptr %arrayidx23, align 4
  %add24 = add nsw i32 %8, %7
  %cmp27 = icmp sgt i32 %add24, %spec.store.select
  %spec.store.select205 = select i1 %cmp27, i32 %add24, i32 %spec.store.select
  store i32 %spec.store.select205, ptr %arrayidx5, align 4
  %arrayidx33 = getelementptr inbounds i32, ptr %bp, i64 %indvars.iv
  %9 = load i32, ptr %arrayidx33, align 4
  %add34 = add nsw i32 %9, %xmb
  %cmp37 = icmp sgt i32 %add34, %spec.store.select205
  %spec.store.select207 = select i1 %cmp37, i32 %add34, i32 %spec.store.select205
  store i32 %spec.store.select207, ptr %arrayidx5, align 4
  %arrayidx43 = getelementptr inbounds i32, ptr %ms, i64 %indvars.iv
  %10 = load i32, ptr %arrayidx43, align 4
  %add46 = add nsw i32 %spec.store.select207, %10
  %cmp49 = icmp slt i32 %add46, -987654321
  %spec.select = select i1 %cmp49, i32 -987654321, i32 %add46
  store i32 %spec.select, ptr %arrayidx5, align 4
  %arrayidx56 = getelementptr inbounds i32, ptr %dc, i64 %2
  %11 = load i32, ptr %arrayidx56, align 4
  %arrayidx59 = getelementptr inbounds i32, ptr %tpdd, i64 %2
  %12 = load i32, ptr %arrayidx59, align 4
  %add60 = add nsw i32 %12, %11
  %arrayidx62 = getelementptr inbounds i32, ptr %dc, i64 %indvars.iv
  store i32 %add60, ptr %arrayidx62, align 4
  %arrayidx65 = getelementptr inbounds i32, ptr %mc, i64 %2
  %13 = load i32, ptr %arrayidx65, align 4
  %arrayidx68 = getelementptr inbounds i32, ptr %tpmd, i64 %2
  %14 = load i32, ptr %arrayidx68, align 4
  %add69 = add nsw i32 %14, %13
  %cmp72 = icmp sgt i32 %add69, %add60
  %spec.store.select208 = select i1 %cmp72, i32 %add69, i32 %add60
  %cmp79 = icmp slt i32 %spec.store.select208, -987654321
  %spec.store.select210 = select i1 %cmp79, i32 -987654321, i32 %spec.store.select208
  store i32 %spec.store.select210, ptr %arrayidx62, align 4
  %cmp84 = icmp ult i64 %indvars.iv, %0
  br i1 %cmp84, label %if.then85, label %for.inc

if.then85:                                        ; preds = %for.body
  %arrayidx87 = getelementptr inbounds i32, ptr %mpp, i64 %indvars.iv
  %15 = load i32, ptr %arrayidx87, align 4
  %arrayidx89 = getelementptr inbounds i32, ptr %tpmi, i64 %indvars.iv
  %16 = load i32, ptr %arrayidx89, align 4
  %add90 = add nsw i32 %16, %15
  %arrayidx92 = getelementptr inbounds i32, ptr %ic, i64 %indvars.iv
  store i32 %add90, ptr %arrayidx92, align 4
  %arrayidx94 = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv
  %17 = load i32, ptr %arrayidx94, align 4
  %arrayidx96 = getelementptr inbounds i32, ptr %tpii, i64 %indvars.iv
  %18 = load i32, ptr %arrayidx96, align 4
  %add97 = add nsw i32 %18, %17
  %cmp100 = icmp sgt i32 %add97, %add90
  %spec.store.select204 = select i1 %cmp100, i32 %add97, i32 %add90
  store i32 %spec.store.select204, ptr %arrayidx92, align 4
  %arrayidx106 = getelementptr inbounds i32, ptr %is, i64 %indvars.iv
  %19 = load i32, ptr %arrayidx106, align 4
  %add109 = add nsw i32 %spec.store.select204, %19
  %cmp112 = icmp slt i32 %add109, -987654321
  %spec.store.select206 = select i1 %cmp112, i32 -987654321, i32 %add109
  store i32 %spec.store.select206, ptr %arrayidx92, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then85, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry.for.end_crit_edge
  %idxprom118.pre-phi = phi i64 [ %.pre, %entry.for.end_crit_edge ], [ %0, %for.end.loopexit ]
  %arrayidx119 = getelementptr inbounds i32, ptr %mc, i64 %idxprom118.pre-phi
  %20 = load i32, ptr %arrayidx119, align 4
  %idxprom120 = sext i32 %n2 to i64
  %arrayidx121 = getelementptr inbounds i32, ptr %dc, i64 %idxprom120
  %21 = load i32, ptr %arrayidx121, align 4
  %add122 = add nsw i32 %21, %20
  %idxprom123 = sext i32 %n3 to i64
  %arrayidx124 = getelementptr inbounds i32, ptr %ic, i64 %idxprom123
  %22 = load i32, ptr %arrayidx124, align 4
  %add125 = add nsw i32 %add122, %22
  %arrayidx127 = getelementptr inbounds i32, ptr %dsq, i64 %idxprom120
  store i32 %add125, ptr %arrayidx127, align 4
  %conv = sitofp i32 %add125 to float
  ret float %conv
}

attributes #0 = { "target-features"="+sse4.2" }

