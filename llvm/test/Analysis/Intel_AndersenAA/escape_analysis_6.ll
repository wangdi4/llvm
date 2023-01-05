; RUN: opt < %s -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 2
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 2
; Non-Escape-Static-Vars_Begin
; foo.fooBuf<mem>
; foo.init<mem>
; Non-Escape-Static-Vars_End
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.init = internal unnamed_addr global i1 false
@foo.fooBuf = internal unnamed_addr global [2 x [1024 x i32]]* null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %fooPtr, i32 %aconst, i32 %n, [2 x [1024 x i32]]** nocapture %bf) #0 {
entry:
  %.b = load i1, i1* @foo.init, align 1
  br i1 %.b, label %entry.if.end_crit_edge, label %if.then

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i64, i64* bitcast ([2 x [1024 x i32]]** @foo.fooBuf to i64*), align 8, !tbaa !1
  br label %if.end

if.then:                                          ; preds = %entry
  store [2 x [1024 x i32]]* bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  store i1 true, i1* @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %entry.if.end_crit_edge, %if.then
  %0 = phi i64 [ %.pre, %entry.if.end_crit_edge ], [ ptrtoint ([2048 x i32]* @foo.local_fooBuf to i64), %if.then ]
  %1 = bitcast [2 x [1024 x i32]]** %bf to i64*
  store i64 %0, i64* %1, align 8, !tbaa !1
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %idxprom1 = sext i32 %n to i64
  %2 = load [2 x [1024 x i32]]*, [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  %arrayidx2 = getelementptr inbounds [2 x [1024 x i32]], [2 x [1024 x i32]]* %2, i64 0, i64 %idxprom1, i64 %idxprom
  %cmp3.15 = icmp sgt i32 %aconst, 0
  br i1 %cmp3.15, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end
  %3 = add i32 %aconst, -1
  %xtraiter = and i32 %aconst, 3
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.preheader.split, label %for.body.prol.preheader

for.body.prol.preheader:                          ; preds = %for.body.preheader
  br label %for.body.prol

for.body.prol:                                    ; preds = %for.body.prol.preheader, %for.body.prol
  %indvars.iv.prol = phi i64 [ %indvars.iv.next.prol, %for.body.prol ], [ 0, %for.body.prol.preheader ]
  %sum.016.prol = phi i32 [ %add.prol, %for.body.prol ], [ undef, %for.body.prol.preheader ]
  %prol.iter = phi i32 [ %prol.iter.sub, %for.body.prol ], [ %xtraiter, %for.body.prol.preheader ]
  %arrayidx5.prol = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.prol
  %4 = trunc i64 %indvars.iv.prol to i32
  store i32 %4, i32* %arrayidx5.prol, align 4, !tbaa !5
  %5 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add.prol = add nsw i32 %5, %sum.016.prol
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i32 %prol.iter, -1
  %prol.iter.cmp = icmp eq i32 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body.preheader.split.loopexit, label %for.body.prol, !llvm.loop !7

for.body.preheader.split.loopexit:                ; preds = %for.body.prol
  %indvars.iv.next.prol.lcssa = phi i64 [ %indvars.iv.next.prol, %for.body.prol ]
  %add.prol.lcssa = phi i32 [ %add.prol, %for.body.prol ]
  br label %for.body.preheader.split

for.body.preheader.split:                         ; preds = %for.body.preheader.split.loopexit, %for.body.preheader
  %add.lcssa.unr = phi i32 [ undef, %for.body.preheader ], [ %add.prol.lcssa, %for.body.preheader.split.loopexit ]
  %indvars.iv.unr = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next.prol.lcssa, %for.body.preheader.split.loopexit ]
  %sum.016.unr = phi i32 [ undef, %for.body.preheader ], [ %add.prol.lcssa, %for.body.preheader.split.loopexit ]
  %6 = icmp ult i32 %3, 3
  br i1 %6, label %for.end.loopexit, label %for.body.preheader.split.split

for.body.preheader.split.split:                   ; preds = %for.body.preheader.split
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader.split.split
  %indvars.iv = phi i64 [ %indvars.iv.unr, %for.body.preheader.split.split ], [ %indvars.iv.next.3, %for.body ]
  %sum.016 = phi i32 [ %sum.016.unr, %for.body.preheader.split.split ], [ %add.3, %for.body ]
  %arrayidx5 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %7 = trunc i64 %indvars.iv to i32
  store i32 %7, i32* %arrayidx5, align 4, !tbaa !5
  %8 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add = add nsw i32 %8, %sum.016
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5.1 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next
  %9 = trunc i64 %indvars.iv.next to i32
  store i32 %9, i32* %arrayidx5.1, align 4, !tbaa !5
  %10 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add.1 = add nsw i32 %10, %add
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %arrayidx5.2 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.1
  %11 = trunc i64 %indvars.iv.next.1 to i32
  store i32 %11, i32* %arrayidx5.2, align 4, !tbaa !5
  %12 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add.2 = add nsw i32 %12, %add.1
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %arrayidx5.3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.2
  %13 = trunc i64 %indvars.iv.next.2 to i32
  store i32 %13, i32* %arrayidx5.3, align 4, !tbaa !5
  %14 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add.3 = add nsw i32 %14, %add.2
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %lftr.wideiv.3 = trunc i64 %indvars.iv.next.3 to i32
  %exitcond.3 = icmp eq i32 %lftr.wideiv.3, %aconst
  br i1 %exitcond.3, label %for.end.loopexit.unr-lcssa, label %for.body

for.end.loopexit.unr-lcssa:                       ; preds = %for.body
  %add.3.lcssa = phi i32 [ %add.3, %for.body ]
  br label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body.preheader.split, %for.end.loopexit.unr-lcssa
  %add.lcssa = phi i32 [ %add.lcssa.unr, %for.body.preheader.split ], [ %add.3.lcssa, %for.end.loopexit.unr-lcssa ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %if.end
  %sum.0.lcssa = phi i32 [ undef, %if.end ], [ %add.lcssa, %for.end.loopexit ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1453)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.disable"}
