; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cond-ldst-motion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the optimization hoisted the loads even if
; the inner Else branch was missing in the outer Else. It was created from
; the following test case:

; int foo(int *a, int n, int m) {
;   int res = 0;
;   for(int i = 0; i < n; i++) {
;     if (m + i == 2) {
;       res += a[i];
;     } else {
;       if (m * i == 5) {
;         res += 2 + a[i];
;       }
;       res += 1 + a[i];
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
;       |      %res.027 = (%a)[i1]  +  %res.027;
;       |   }
;       |   else
;       |   {
;       |      if (%m * i1 == 5)
;       |      {
;       |         %4 = (%a)[i1];
;       |         %res.027 = %4 + 2  +  %res.027;
;       |      }
;       |      %5 = (%a)[i1];
;       |      %res.027 = %5 + 1  +  %res.027;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %cldst.hoisted = (%a)[i1];
; CHECK:       |   if (i1 + sext.i32.i64(%m) == 2)
; CHECK:       |   {
; CHECK:       |      %res.027 = %cldst.hoisted  +  %res.027;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      if (%m * i1 == 5)
; CHECK:       |      {
; CHECK:       |         %4 = %cldst.hoisted;
; CHECK:       |         %res.027 = %4 + 2  +  %res.027;
; CHECK:       |      }
; CHECK:       |      %5 = %cldst.hoisted;
; CHECK:       |      %res.027 = %5 + 1  +  %res.027;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local noundef i32 @_Z3fooPiii(ptr nocapture noundef readonly %a, i32 noundef %n, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %m to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.2.lcssa = phi i32 [ %res.2, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %res.2.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %i.028 = phi i32 [ 0, %for.body.preheader ], [ %inc, %for.inc ]
  %res.027 = phi i32 [ 0, %for.body.preheader ], [ %res.2, %for.inc ]
  %1 = add nsw i64 %indvars.iv, %0
  %2 = icmp eq i64 %1, 2
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %add2 = add nsw i32 %3, %res.027
  br label %for.inc

if.else:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.028, %m
  %cmp3 = icmp eq i32 %mul, 5
  br i1 %cmp3, label %if.then4, label %if.end

if.then4:                                         ; preds = %if.else
  %arrayidx6 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx6, align 4, !tbaa !3
  %add7 = add nsw i32 %4, 2
  %add8 = add nsw i32 %add7, %res.027
  br label %if.end

if.end:                                           ; preds = %if.then4, %if.else
  %res.1 = phi i32 [ %add8, %if.then4 ], [ %res.027, %if.else ]
  %arrayidx10 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %5 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  %add11 = add nsw i32 %5, 1
  %add12 = add nsw i32 %add11, %res.1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.end
  %res.2 = phi i32 [ %add2, %if.then ], [ %add12, %if.end ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i.028, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !7
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

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
