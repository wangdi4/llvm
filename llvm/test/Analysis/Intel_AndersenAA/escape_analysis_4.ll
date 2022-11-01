; RUN: opt < %s -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 1
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 1
; Non-Escape-Static-Vars_Begin
; foo.fooBuf<mem>
; Non-Escape-Static-Vars_End
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@init = global i32 1, align 4
@sum = global i32 0, align 4
@foo.fooBuf = internal unnamed_addr global i32* null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32* @foo(i32* nocapture %fooPtr, i32 %aconst) #0 {
entry:
  %0 = load i32, i32* @init, align 4, !tbaa !1
  %cmp = icmp eq i32 %0, 1
  br i1 %cmp, label %if.then, label %entry.if.end_crit_edge

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i32*, i32** @foo.fooBuf, align 8, !tbaa !5
  br label %if.end

if.then:                                          ; preds = %entry
  store i32* getelementptr inbounds ([2048 x i32], [2048 x i32]* @foo.local_fooBuf, i64 0, i64 0), i32** @foo.fooBuf, align 8, !tbaa !5
  store i32 0, i32* @init, align 4, !tbaa !1
  br label %if.end

if.end:                                           ; preds = %entry.if.end_crit_edge, %if.then
  %1 = phi i32* [ %.pre, %entry.if.end_crit_edge ], [ getelementptr inbounds ([2048 x i32], [2048 x i32]* @foo.local_fooBuf, i64 0, i64 0), %if.then ]
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 %idxprom
  %cmp1.11 = icmp sgt i32 %aconst, 0
  br i1 %cmp1.11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end
  %2 = add i32 %aconst, -1
  %xtraiter = and i32 %aconst, 3
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.preheader.split, label %for.body.prol.preheader

for.body.prol.preheader:                          ; preds = %for.body.preheader
  br label %for.body.prol

for.body.prol:                                    ; preds = %for.body.prol.preheader, %for.body.prol
  %indvars.iv.prol = phi i64 [ %indvars.iv.next.prol, %for.body.prol ], [ 0, %for.body.prol.preheader ]
  %prol.iter = phi i32 [ %prol.iter.sub, %for.body.prol ], [ %xtraiter, %for.body.prol.preheader ]
  %arrayidx3.prol = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.prol
  %3 = trunc i64 %indvars.iv.prol to i32
  store i32 %3, i32* %arrayidx3.prol, align 4, !tbaa !1
  %4 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %5 = load i32, i32* @sum, align 4, !tbaa !1
  %add.prol = add nsw i32 %5, %4
  store i32 %add.prol, i32* @sum, align 4, !tbaa !1
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i32 %prol.iter, -1
  %prol.iter.cmp = icmp eq i32 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body.preheader.split.loopexit, label %for.body.prol, !llvm.loop !7

for.body.preheader.split.loopexit:                ; preds = %for.body.prol
  %indvars.iv.next.prol.lcssa = phi i64 [ %indvars.iv.next.prol, %for.body.prol ]
  br label %for.body.preheader.split

for.body.preheader.split:                         ; preds = %for.body.preheader.split.loopexit, %for.body.preheader
  %indvars.iv.unr = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next.prol.lcssa, %for.body.preheader.split.loopexit ]
  %6 = icmp ult i32 %2, 3
  br i1 %6, label %for.end.loopexit, label %for.body.preheader.split.split

for.body.preheader.split.split:                   ; preds = %for.body.preheader.split
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader.split.split
  %indvars.iv = phi i64 [ %indvars.iv.unr, %for.body.preheader.split.split ], [ %indvars.iv.next.3, %for.body ]
  %arrayidx3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %7 = trunc i64 %indvars.iv to i32
  store i32 %7, i32* %arrayidx3, align 4, !tbaa !1
  %8 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %9 = load i32, i32* @sum, align 4, !tbaa !1
  %add = add nsw i32 %9, %8
  store i32 %add, i32* @sum, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx3.1 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next
  %10 = trunc i64 %indvars.iv.next to i32
  store i32 %10, i32* %arrayidx3.1, align 4, !tbaa !1
  %11 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %12 = load i32, i32* @sum, align 4, !tbaa !1
  %add.1 = add nsw i32 %12, %11
  store i32 %add.1, i32* @sum, align 4, !tbaa !1
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %arrayidx3.2 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.1
  %13 = trunc i64 %indvars.iv.next.1 to i32
  store i32 %13, i32* %arrayidx3.2, align 4, !tbaa !1
  %14 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %15 = load i32, i32* @sum, align 4, !tbaa !1
  %add.2 = add nsw i32 %15, %14
  store i32 %add.2, i32* @sum, align 4, !tbaa !1
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %arrayidx3.3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.2
  %16 = trunc i64 %indvars.iv.next.2 to i32
  store i32 %16, i32* %arrayidx3.3, align 4, !tbaa !1
  %17 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %18 = load i32, i32* @sum, align 4, !tbaa !1
  %add.3 = add nsw i32 %18, %17
  store i32 %add.3, i32* @sum, align 4, !tbaa !1
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %lftr.wideiv.3 = trunc i64 %indvars.iv.next.3 to i32
  %exitcond.3 = icmp eq i32 %lftr.wideiv.3, %aconst
  br i1 %exitcond.3, label %for.end.loopexit.unr-lcssa, label %for.body

for.end.loopexit.unr-lcssa:                       ; preds = %for.body
  br label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body.preheader.split, %for.end.loopexit.unr-lcssa
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %if.end
  ret i32* getelementptr inbounds ([2048 x i32], [2048 x i32]* @foo.local_fooBuf, i64 0, i64 0)
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
