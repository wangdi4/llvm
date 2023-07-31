; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s
;
;  We move out all the three stores out of i2 loop and mark %0 as non-linear because it is defined inside i1 loop.
;
; Source code:
;int b, c, d, e;
;int f[100];
;int g() {
;  for (; e; e++) {
;    for (; c; c += 2) {
;      d = 2;
;      for (; d < 10; d += 3)
;        f[d] = b;
;    }
;    b = (int) f;
;  }
;  return 0;
;}
;
;*** IR Dump Before HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { modified }
;<48>               + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
;<2>                |   %c.promoted.out = %c.promoted;
;<4>                |   %.pr1117 = 0;
;<5>                |   if (%.pr11 != 0)
;<5>                |   {
;<49>               |      + DO i2 = 0, (-1 * %c.promoted.out + -2)/u2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483648>
;<52>               |      |   (@f)[0][2] = %0;
;<53>               |      |   (@f)[0][5] = %0;
;<18>               |      |   (@f)[0][8] = %0;
;<49>               |      + END LOOP
;<49>               |
;<34>               |      %c.promoted = %c.promoted.out + 2  +  2 * ((-2 + (-1 * %c.promoted.out)) /u 2);
;<35>               |      (@d)[0] = 11;
;<36>               |      (@c)[0] = %c.promoted;
;<37>               |      %.pr1117 = %c.promoted;
;<5>                |   }
;<42>               |   %.pr11 = %.pr1117;
;<43>               |   %0 = ptrtoint ([100 x i32]* @f to i32);
;<48>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Memory Motion ***
;
;<0>          BEGIN REGION { modified }
;<48>               + DO i1 = 0, -1 * %.pr + -1, 1   <DO_LOOP>
;<2>                |   %c.promoted.out = %c.promoted;
;<4>                |   %.pr1117 = 0;
;<5>                |   if (%.pr11 != 0)
;<5>                |   {
;<18>               |      (@f)[0][8] = %0;
;<53>               |      (@f)[0][5] = %0;
;<52>               |      (@f)[0][2] = %0;
;<34>               |      %c.promoted = %c.promoted.out + 2  +  2 * ((-2 + (-1 * %c.promoted.out)) /u 2);
;<35>               |      (@d)[0] = 11;
;<36>               |      (@c)[0] = %c.promoted;
;<37>               |      %.pr1117 = %c.promoted;
;<5>                |   }
;<42>               |   %.pr11 = %.pr1117;
;<43>               |   %0 = ptrtoint ([100 x i32]* @f to i32);
;<48>               + END LOOP
;<0>          END REGION
;
;CHECK:   (@f)[0][8] = %0;
;CHECK:   <RVAL-REG> NON-LINEAR i32 %0
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e = common dso_local local_unnamed_addr global i32 0, align 4
@c = common dso_local local_unnamed_addr global i32 0, align 4
@d = common dso_local local_unnamed_addr global i32 0, align 4
@b = common dso_local local_unnamed_addr global i32 0, align 4
@f = common dso_local global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @g() local_unnamed_addr #0 {
entry:
  %.pr = load i32, ptr @e, align 4, !tbaa !2
  %tobool15 = icmp eq i32 %.pr, 0
  br i1 %tobool15, label %for.end10, label %for.cond1thread-pre-split.lr.ph

for.cond1thread-pre-split.lr.ph:                  ; preds = %entry
  %b.promoted = load i32, ptr @b, align 4, !tbaa !2
  %.pr11.pre = load i32, ptr @c, align 4, !tbaa !2
  br label %for.cond1thread-pre-split

for.cond1thread-pre-split:                        ; preds = %for.cond1thread-pre-split.lr.ph, %for.end8
  %c.promoted = phi i32 [ %.pr11.pre, %for.cond1thread-pre-split.lr.ph ], [ %c.promoted19, %for.end8 ]
  %.pr11 = phi i32 [ %.pr11.pre, %for.cond1thread-pre-split.lr.ph ], [ %.pr1117, %for.end8 ]
  %inc16 = phi i32 [ %.pr, %for.cond1thread-pre-split.lr.ph ], [ %inc, %for.end8 ]
  %0 = phi i32 [ %b.promoted, %for.cond1thread-pre-split.lr.ph ], [ ptrtoint (ptr @f to i32), %for.end8 ]
  %tobool213 = icmp eq i32 %.pr11, 0
  br i1 %tobool213, label %for.end8, label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1thread-pre-split
  %1 = sub i32 -2, %c.promoted
  %2 = and i32 %1, -2
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc6
  %add714 = phi i32 [ %c.promoted, %for.cond4.preheader.lr.ph ], [ %add7, %for.inc6 ]
  br label %for.body5

for.body5:                                        ; preds = %for.cond4.preheader, %for.body5
  %indvars.iv = phi i64 [ 2, %for.cond4.preheader ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @f, i64 0, i64 %indvars.iv, !intel-tbaa !6
  store i32 %0, ptr %arrayidx, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 10
  br i1 %cmp, label %for.body5, label %for.inc6

for.inc6:                                         ; preds = %for.body5
  %add7 = add nsw i32 %add714, 2
  %tobool2 = icmp eq i32 %add7, 0
  br i1 %tobool2, label %for.cond1.for.end8_crit_edge, label %for.cond4.preheader

for.cond1.for.end8_crit_edge:                     ; preds = %for.inc6
  %3 = add i32 %c.promoted, 2
  %4 = add i32 %3, %2
  store i32 11, ptr @d, align 4, !tbaa !2
  store i32 %4, ptr @c, align 4, !tbaa !2
  br label %for.end8

for.end8:                                         ; preds = %for.cond1.for.end8_crit_edge, %for.cond1thread-pre-split
  %c.promoted19 = phi i32 [ %4, %for.cond1.for.end8_crit_edge ], [ %c.promoted, %for.cond1thread-pre-split ]
  %.pr1117 = phi i32 [ %4, %for.cond1.for.end8_crit_edge ], [ 0, %for.cond1thread-pre-split ]
  %inc = add nsw i32 %inc16, 1
  %tobool = icmp eq i32 %inc, 0
  br i1 %tobool, label %for.cond.for.end10_crit_edge, label %for.cond1thread-pre-split

for.cond.for.end10_crit_edge:                     ; preds = %for.end8
  store i32 ptrtoint (ptr @f to i32), ptr @b, align 4, !tbaa !2
  store i32 0, ptr @e, align 4, !tbaa !2
  br label %for.end10

for.end10:                                        ; preds = %for.cond.for.end10_crit_edge, %entry
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang b97cd1e0ccdf66edc0b2a4aadd0de0874ecd119f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ab9ae02b1b742d798e1eaffbfb47060105577dc5)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_i", !3, i64 0}
