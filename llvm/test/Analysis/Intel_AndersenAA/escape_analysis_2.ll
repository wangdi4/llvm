; RUN: opt < %s -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 3
; RUN: opt < %s -passes=convert-to-subscript -S | opt -passes='require<anders-aa>' -print-non-escape-candidates -disable-output 2>&1 | grep "foo." | count 3
; Non-Escape-Static-Vars_Begin
; foo.fooBuf<mem>
; foo.init<mem>
; foo.local_fooBuf<mem>
; Non-Escape-Static-Vars_End
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.init = internal unnamed_addr global i1 false
@foo.fooBuf = internal unnamed_addr global [2 x [1024 x i32]]* null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %fooPtr, i32 %aconst, i32 %n) #0 {
entry:
  %.b = load i1, i1* @foo.init, align 1
  br i1 %.b, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  store [2 x [1024 x i32]]* bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  store i1 true, i1* @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %cmp3.15 = icmp sgt i32 %aconst, 0
  br i1 %cmp3.15, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %if.end
  %0 = load [2 x [1024 x i32]]*, [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  %idxprom1 = sext i32 %n to i64
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %arrayidx2 = getelementptr inbounds [2 x [1024 x i32]], [2 x [1024 x i32]]* %0, i64 0, i64 %idxprom1, i64 %idxprom
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %2 = add i32 %aconst, -1
  %xtraiter = and i32 %aconst, 7
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.body.preheader.split, label %for.body.prol

for.body.prol:                                    ; preds = %for.body.prol, %for.body.preheader
  %indvars.iv.prol = phi i64 [ %indvars.iv.next.prol, %for.body.prol ], [ 0, %for.body.preheader ]
  %prol.iter = phi i32 [ %prol.iter.sub, %for.body.prol ], [ %xtraiter, %for.body.preheader ]
  %arrayidx5.prol = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.prol
  %3 = trunc i64 %indvars.iv.prol to i32
  store i32 %3, i32* %arrayidx5.prol, align 4, !tbaa !5
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i32 %prol.iter, -1
  %prol.iter.cmp = icmp eq i32 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.body.preheader.split, label %for.body.prol, !llvm.loop !7

for.body.preheader.split:                         ; preds = %for.body.prol, %for.body.preheader
  %indvars.iv.unr = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next.prol, %for.body.prol ]
  %4 = icmp ult i32 %2, 7
  br i1 %4, label %for.end.loopexit, label %for.body

for.body:                                         ; preds = %for.body.preheader.split, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next.7, %for.body ], [ %indvars.iv.unr, %for.body.preheader.split ]
  %arrayidx5 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %5 = trunc i64 %indvars.iv to i32
  store i32 %5, i32* %arrayidx5, align 4, !tbaa !5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv1 = trunc i64 %indvars.iv.next to i32
  %arrayidx5.1 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next
  store i32 %lftr.wideiv1, i32* %arrayidx5.1, align 4, !tbaa !5
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %lftr.wideiv1.1 = trunc i64 %indvars.iv.next.1 to i32
  %arrayidx5.2 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.1
  store i32 %lftr.wideiv1.1, i32* %arrayidx5.2, align 4, !tbaa !5
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %lftr.wideiv1.2 = trunc i64 %indvars.iv.next.2 to i32
  %arrayidx5.3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.2
  store i32 %lftr.wideiv1.2, i32* %arrayidx5.3, align 4, !tbaa !5
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %lftr.wideiv1.3 = trunc i64 %indvars.iv.next.3 to i32
  %arrayidx5.4 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.3
  store i32 %lftr.wideiv1.3, i32* %arrayidx5.4, align 4, !tbaa !5
  %indvars.iv.next.4 = add nsw i64 %indvars.iv, 5
  %lftr.wideiv1.4 = trunc i64 %indvars.iv.next.4 to i32
  %arrayidx5.5 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.4
  store i32 %lftr.wideiv1.4, i32* %arrayidx5.5, align 4, !tbaa !5
  %indvars.iv.next.5 = add nsw i64 %indvars.iv, 6
  %lftr.wideiv1.5 = trunc i64 %indvars.iv.next.5 to i32
  %arrayidx5.6 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.5
  store i32 %lftr.wideiv1.5, i32* %arrayidx5.6, align 4, !tbaa !5
  %indvars.iv.next.6 = add nsw i64 %indvars.iv, 7
  %lftr.wideiv1.6 = trunc i64 %indvars.iv.next.6 to i32
  %arrayidx5.7 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.6
  store i32 %lftr.wideiv1.6, i32* %arrayidx5.7, align 4, !tbaa !5
  %indvars.iv.next.7 = add nsw i64 %indvars.iv, 8
  %lftr.wideiv1.7 = trunc i64 %indvars.iv.next.7 to i32
  %exitcond2.7 = icmp eq i32 %lftr.wideiv1.7, %aconst
  br i1 %exitcond2.7, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body, %for.body.preheader.split
  %6 = mul i32 %1, %aconst
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %if.end
  %sum.0.lcssa = phi i32 [ 0, %if.end ], [ %6, %for.end.loopexit ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1439)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.disable"}
