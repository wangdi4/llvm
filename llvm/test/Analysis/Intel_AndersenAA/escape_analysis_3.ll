; RUN: opt < %s -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | FileCheck %s
; CHECK: Non-Escape-Static-Vars_Begin
; CHECK-NEXT: Non-Escape-Static-Vars_End
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@init = global i32 1, align 4
@local_fooBuf = global [2048 x i32] zeroinitializer, align 16
@fooBuf = common global i32* null, align 8

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %fooPtr, i32 %aconst) #0 {
entry:
  %0 = load i32, i32* @init, align 4, !tbaa !1
  %cmp = icmp eq i32 %0, 1
  br i1 %cmp, label %if.then, label %entry.if.end_crit_edge

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i32*, i32** @fooBuf, align 8, !tbaa !5
  br label %if.end

if.then:                                          ; preds = %entry
  store i32* getelementptr inbounds ([2048 x i32], [2048 x i32]* @local_fooBuf, i64 0, i64 0), i32** @fooBuf, align 8, !tbaa !5
  store i32 0, i32* @init, align 4, !tbaa !1
  br label %if.end

if.end:                                           ; preds = %entry.if.end_crit_edge, %if.then
  %1 = phi i32* [ %.pre, %entry.if.end_crit_edge ], [ getelementptr inbounds ([2048 x i32], [2048 x i32]* @local_fooBuf, i64 0, i64 0), %if.then ]
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 %idxprom
  %cmp1.13 = icmp sgt i32 %aconst, 0
  br i1 %cmp1.13, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end
  %2 = add i32 %aconst, -1
  %xtraiter = and i32 %aconst, 3
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.preheader.split, label %for.body.prol.preheader

for.body.prol.preheader:                          ; preds = %for.body.preheader
  %3 = and i32 %aconst, 3
  %4 = add nsw i32 %3, -1
  %5 = zext i32 %4 to i64
  br label %for.body.prol

for.body.prol:                                    ; preds = %for.body.prol.preheader, %for.body.prol
  %indvars.iv.prol = phi i64 [ %indvars.iv.next.prol, %for.body.prol ], [ 0, %for.body.prol.preheader ]
  %sum.014.prol = phi i32 [ %add.prol, %for.body.prol ], [ 0, %for.body.prol.preheader ]
  %prol.iter = phi i32 [ %prol.iter.sub, %for.body.prol ], [ %xtraiter, %for.body.prol.preheader ]
  %arrayidx3.prol = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.prol
  %6 = trunc i64 %indvars.iv.prol to i32
  store i32 %6, i32* %arrayidx3.prol, align 4, !tbaa !1
  %7 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add.prol = add nsw i32 %7, %sum.014.prol
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i32 %prol.iter, -1
  %prol.iter.cmp = icmp eq i32 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body.preheader.split.loopexit, label %for.body.prol, !llvm.loop !7

for.body.preheader.split.loopexit:                ; preds = %for.body.prol
  %8 = add nuw nsw i64 %5, 1
  br label %for.body.preheader.split

for.body.preheader.split:                         ; preds = %for.body.preheader.split.loopexit, %for.body.preheader
  %add.lcssa.unr = phi i32 [ undef, %for.body.preheader ], [ %add.prol, %for.body.preheader.split.loopexit ]
  %indvars.iv.unr = phi i64 [ 0, %for.body.preheader ], [ %8, %for.body.preheader.split.loopexit ]
  %sum.014.unr = phi i32 [ 0, %for.body.preheader ], [ %add.prol, %for.body.preheader.split.loopexit ]
  %9 = icmp ult i32 %2, 3
  br i1 %9, label %for.end, label %for.body

for.body:                                         ; preds = %for.body.preheader.split, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next.3, %for.body ], [ %indvars.iv.unr, %for.body.preheader.split ]
  %sum.014 = phi i32 [ %add.3, %for.body ], [ %sum.014.unr, %for.body.preheader.split ]
  %arrayidx3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %10 = trunc i64 %indvars.iv to i32
  store i32 %10, i32* %arrayidx3, align 4, !tbaa !1
  %11 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %11, %sum.014
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx3.1 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next
  %12 = trunc i64 %indvars.iv.next to i32
  store i32 %12, i32* %arrayidx3.1, align 4, !tbaa !1
  %13 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add.1 = add nsw i32 %add, %13
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %arrayidx3.2 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.1
  %14 = trunc i64 %indvars.iv.next.1 to i32
  store i32 %14, i32* %arrayidx3.2, align 4, !tbaa !1
  %15 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add.2 = add nsw i32 %add.1, %15
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %arrayidx3.3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.2
  %16 = trunc i64 %indvars.iv.next.2 to i32
  store i32 %16, i32* %arrayidx3.3, align 4, !tbaa !1
  %17 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add.3 = add nsw i32 %add.2, %17
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %lftr.wideiv.3 = trunc i64 %indvars.iv.next.3 to i32
  %exitcond.3 = icmp eq i32 %lftr.wideiv.3, %aconst
  br i1 %exitcond.3, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %for.body.preheader.split, %if.end
  %sum.0.lcssa = phi i32 [ 0, %if.end ], [ %add.lcssa.unr, %for.body.preheader.split ], [ %add.3, %for.body ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1453)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !3, i64 0}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.disable"}
