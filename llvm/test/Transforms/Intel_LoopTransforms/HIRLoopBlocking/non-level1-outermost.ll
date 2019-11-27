; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-blocking -print-after=hir-loop-blocking -print-before=hir-loop-blocking  < %s 2>&1 | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa"  2>&1 < %s | FileCheck %s --check-prefix=DEFAULT

;typedef long long a[1];
;typedef a b[9];
;unsigned c, e;
;int d;
;long long f;
;b g;
;void h() {
;  for (; d; d++)
;    for (e = 2; e; e += 2)
;      for (c = 2; c; c += 2)
;        g[1][c] = f;
;}

; This test makes sure, the outermost loop of blocking can be a loop at level larger than 1.

; DEFAULT:Function: h

; DEFAULT:        BEGIN REGION { }
; DEFAULT:              + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
; DEFAULT:              |   + DO i2 = 0, 2147483646, 1   <DO_LOOP>
; DEFAULT:              |   |   + DO i3 = 0, 2147483646, 1   <DO_LOOP>
; DEFAULT:              |   |   |   (@g)[0][1][2 * i3 + 2] = %0;
; DEFAULT:              |   |   + END LOOP
; DEFAULT:              |   + END LOOP
; DEFAULT:              + END LOOP
; DEFAULT:        END REGION

; DEFAULT:Function: h

; DEFAULT:        BEGIN REGION { modified }
; DEFAULT:              + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
; DEFAULT:              |   + DO i2 = 0, 33554431, 1   <DO_LOOP>
; DEFAULT:              |   |   %min = (-64 * i2 + 2147483646 <= 63) ? -64 * i2 + 2147483646 : 63;
; DEFAULT:              |   |
; DEFAULT:              |   |   + DO i3 = 0, 33554431, 1   <DO_LOOP>
; DEFAULT:              |   |   |   %min3 = (-64 * i3 + 2147483646 <= 63) ? -64 * i3 + 2147483646 : 63;
; DEFAULT:              |   |   |
; DEFAULT:              |   |   |   + DO i4 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; DEFAULT:              |   |   |   |   + DO i5 = 0, %min3, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; DEFAULT:              |   |   |   |   |   (@g)[0][1][128 * i3 + 2 * i5 + 2] = %0;
; DEFAULT:              |   |   |   |   + END LOOP
; DEFAULT:              |   |   |   + END LOOP
; DEFAULT:              |   |   + END LOOP
; DEFAULT:              |   + END LOOP
; DEFAULT:              + END LOOP
; DEFAULT:        END REGION

;Module Before HIR
; ModuleID = 'non-level1-outermost.c'
source_filename = "non-level1-outermost.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = common dso_local local_unnamed_addr global i32 0, align 4
@e = common dso_local local_unnamed_addr global i32 0, align 4
@c = common dso_local local_unnamed_addr global i32 0, align 4
@f = common dso_local local_unnamed_addr global i64 0, align 8
@g = common dso_local local_unnamed_addr global [9 x [1 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @h() local_unnamed_addr #0 {
entry:
  %.pr = load i32, i32* @d, align 4, !tbaa !2
  %tobool15 = icmp eq i32 %.pr, 0
  br i1 %tobool15, label %for.end11, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = load i64, i64* @f, align 8, !tbaa !6
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc10
  %inc16 = phi i32 [ %.pr, %for.cond1.preheader.lr.ph ], [ %inc, %for.inc10 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc7
  %storemerge14 = phi i32 [ 2, %for.cond1.preheader ], [ %add8, %for.inc7 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 2, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx = getelementptr inbounds [9 x [1 x i64]], [9 x [1 x i64]]* @g, i64 0, i64 1, i64 %indvars.iv
  store i64 %0, i64* %arrayidx, align 8, !tbaa !8
  %1 = trunc i64 %indvars.iv to i32
  %tobool5 = icmp eq i32 %1, -2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  br i1 %tobool5, label %for.inc7, label %for.body6

for.inc7:                                         ; preds = %for.body6
  %add8 = add i32 %storemerge14, 2
  %tobool2 = icmp eq i32 %add8, 0
  br i1 %tobool2, label %for.inc10, label %for.cond4.preheader

for.inc10:                                        ; preds = %for.inc7
  %inc = add nsw i32 %inc16, 1
  %tobool = icmp eq i32 %inc, 0
  br i1 %tobool, label %for.cond.for.end11_crit_edge, label %for.cond1.preheader

for.cond.for.end11_crit_edge:                     ; preds = %for.inc10
  store i32 0, i32* @c, align 4, !tbaa !2
  store i32 0, i32* @e, align 4, !tbaa !2
  store i32 0, i32* @d, align 4, !tbaa !2
  br label %for.end11

for.end11:                                        ; preds = %for.cond.for.end11_crit_edge, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"long long", !4, i64 0}
!8 = !{!9, !7, i64 0}
!9 = !{!"array@_ZTSA9_A1_x", !10, i64 0}
!10 = !{!"array@_ZTSA1_x", !7, i64 0}
