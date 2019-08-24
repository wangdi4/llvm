;    for ( i=0; i< n; i++) {
;			s  = a[s] + n;
;    }
;    for ( i=0; i< n; i++) {
;			s  = b[s][i] + n;
;    }
;    for ( i=0; i< n; i++) {
;			s  = s  +   a[i];
;			s  = s  +  	b[i+s][s];
;   }
;
; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s
; CHECK: No Safe Reduction
;
; ModuleID = 'sum17.c'
source_filename = "sum17.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x i64] zeroinitializer, align 16
@b = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i64 %n, i64 %n2, i64 %n3, double* nocapture %res) local_unnamed_addr #0 {
entry:
  %cmp59 = icmp sgt i64 %n, 0
  br i1 %cmp59, label %for.body, label %for.end28

for.cond2.preheader:                              ; preds = %for.body
  %cmp454 = icmp sgt i64 %n, 0
  br i1 %cmp454, label %for.body6, label %for.end28

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv66 = phi i64 [ %indvars.iv.next67, %for.body ], [ 0, %entry ]
  %s.060 = phi i64 [ %add, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %s.060
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !1
  %add = add nsw i64 %0, %n
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond68 = icmp eq i64 %indvars.iv.next67, %n
  br i1 %exitcond68, label %for.cond2.preheader, label %for.body

for.cond13.preheader:                             ; preds = %for.body6
  %cmp1550 = icmp sgt i64 %n, 0
  br i1 %cmp1550, label %for.body17, label %for.end28

for.body6:                                        ; preds = %for.cond2.preheader, %for.body6
  %indvars.iv63 = phi i64 [ %indvars.iv.next64, %for.body6 ], [ 0, %for.cond2.preheader ]
  %s.155 = phi i64 [ %add9, %for.body6 ], [ %add, %for.cond2.preheader ]
  %arrayidx8 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @b, i64 0, i64 %s.155, i64 %indvars.iv63
  %1 = load i64, i64* %arrayidx8, align 8, !tbaa !1
  %add9 = add nsw i64 %1, %n
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond65 = icmp eq i64 %indvars.iv.next64, %n
  br i1 %exitcond65, label %for.cond13.preheader, label %for.body6

for.body17:                                       ; preds = %for.cond13.preheader, %for.body17
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body17 ], [ 0, %for.cond13.preheader ]
  %s.251 = phi i64 [ %add25, %for.body17 ], [ %add9, %for.cond13.preheader ]
  %arrayidx19 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %indvars.iv
  %2 = load i64, i64* %arrayidx19, align 8, !tbaa !1
  %add20 = add nsw i64 %2, %s.251
  %add22 = add nsw i64 %add20, %indvars.iv
  %arrayidx24 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @b, i64 0, i64 %add22, i64 %add20
  %3 = load i64, i64* %arrayidx24, align 8, !tbaa !1
  %add25 = add nsw i64 %add20, %3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end28, label %for.body17

for.end28:                                        ; preds = %for.body17, %entry, %for.cond2.preheader, %for.cond13.preheader
  %s.2.lcssa = phi i64 [ %add9, %for.cond13.preheader ], [ %add, %for.cond2.preheader ], [ 0, %entry ], [ %add25, %for.body17 ]
  %conv29 = sitofp i64 %s.2.lcssa to double
  %4 = load double, double* %res, align 8, !tbaa !5
  %add30 = fadd double %conv29, %4
  store double %add30, double* %res, align 8, !tbaa !5
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17943)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"double", !3, i64 0}
