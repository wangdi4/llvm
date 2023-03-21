; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the transformation can hoist and sink loads
; and stores in the same branches. It was created from the following test case:

; int foo(int *a, int n, int m) {
; 
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       a[i] = a[m];
;       res += (0 + a[i]);
;     } else if (m * i == 5) {
;       a[i] = a[m] + 3;
;       res += (-1 + a[i]);
;     } else {
;       a[i] = a[m] + n;
;       res += (1 + a[i]);
;     }
;   }
; 
;   return res;
; }

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if (i1 + sext.i32.i64(%m) == 2)
;       |   {
;       |      %2 = (%a)[%m];
;       |      (%a)[i1] = %2;
;       |      %.pn = %2;
;       |   }
;       |   else
;       |   {
;       |      if (%m * i1 == 5)
;       |      {
;       |         %3 = (%a)[%m];
;       |         (%a)[i1] = %3 + 3;
;       |         %.pn = %3 + 2;
;       |      }
;       |      else
;       |      {
;       |         %4 = (%a)[%m];
;       |         (%a)[i1] = %n + %4;
;       |         %.pn = %n + %4 + 1;
;       |      }
;       |   }
;       |   %res.056 = %.pn  +  %res.056;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %cldst.hoisted = (%a)[%m];
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %2 = %cldst.hoisted;
; CHECK:       |      %cldst.sunk = %2;
; CHECK:       |      %.pn = %2;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %3 = %cldst.hoisted;
; CHECK:       |         %cldst.sunk = %3 + 3;
; CHECK:       |         %.pn = %3 + 2;
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %4 = %cldst.hoisted;
; CHECK:       |         %cldst.sunk = %n + %4;
; CHECK:       |         %.pn = %n + %4 + 1;
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       |   (%a)[i1] = %cldst.sunk;
; CHECK:       |   %res.056 = %.pn  +  %res.056;
; CHECK:       + END LOOP
; CHECK: END REGION



;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef %a, i32 noundef %n, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp55 = icmp sgt i32 %n, 0
  br i1 %cmp55, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %idxprom20 = sext i32 %m to i64
  %arrayidx21 = getelementptr inbounds i32, ptr %a, i64 %idxprom20
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.1.lcssa = phi i32 [ %res.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %i.057 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %res.056 = phi i32 [ 0, %for.body.lr.ph ], [ %res.1, %for.inc ]
  %0 = add nsw i64 %indvars.iv, %idxprom20
  %1 = icmp eq i64 %0, 2
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %2 = load i32, ptr %arrayidx21, align 4, !tbaa !3
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %2, ptr %arrayidx3, align 4, !tbaa !3
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.057, %m
  %cmp8 = icmp eq i32 %mul, 5
  br i1 %cmp8, label %if.then9, label %if.else19

if.then9:                                         ; preds = %if.else
  %3 = load i32, ptr %arrayidx21, align 4, !tbaa !3
  %add12 = add nsw i32 %3, 3
  %arrayidx14 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %add12, ptr %arrayidx14, align 4, !tbaa !3
  %add17 = add nsw i32 %3, 2
  br label %for.inc

if.else19:                                        ; preds = %if.else
  %4 = load i32, ptr %arrayidx21, align 4, !tbaa !3
  %add22 = add nsw i32 %4, %n
  %arrayidx24 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %add22, ptr %arrayidx24, align 4, !tbaa !3
  %add27 = add nsw i32 %add22, 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else19, %if.then9
  %.pn = phi i32 [ %2, %if.then ], [ %add17, %if.then9 ], [ %add27, %if.else19 ]
  %res.1 = add nsw i32 %.pn, %res.056
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.057, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
