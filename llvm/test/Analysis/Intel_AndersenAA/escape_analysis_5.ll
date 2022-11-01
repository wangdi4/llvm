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
define i32 @foo(i32* nocapture %fooPtr, i32 %aconst, i32 %n) #0 {
entry:
  %.b = load i1, i1* @foo.init, align 1
  br i1 %.b, label %entry.if.end_crit_edge, label %if.then

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load [2 x [1024 x i32]]*, [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  br label %if.end

if.then:                                          ; preds = %entry
  store [2 x [1024 x i32]]* bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  store i1 true, i1* @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %entry.if.end_crit_edge
  %0 = phi [2 x [1024 x i32]]* [ %.pre, %entry.if.end_crit_edge ], [ bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), %if.then ]
  tail call void @bar([2 x [1024 x i32]]* %0) #3
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %idxprom1 = sext i32 %n to i64
  %1 = load [2 x [1024 x i32]]*, [2 x [1024 x i32]]** @foo.fooBuf, align 8, !tbaa !1
  %arrayidx2 = getelementptr inbounds [2 x [1024 x i32]], [2 x [1024 x i32]]* %1, i64 0, i64 %idxprom1, i64 %idxprom
  %cmp3.15 = icmp sgt i32 %aconst, 0
  br i1 %cmp3.15, label %for.body, label %for.end

for.body:                                         ; preds = %for.body, %if.end
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %if.end ]
  %sum.016 = phi i32 [ %add, %for.body ], [ undef, %if.end ]
  %arrayidx5 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx5, align 4, !tbaa !5
  %3 = load i32, i32* %arrayidx2, align 4, !tbaa !5
  %add = add nsw i32 %3, %sum.016
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %aconst
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %if.end
  %sum.0.lcssa = phi i32 [ undef, %if.end ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @bar([2 x [1024 x i32]]*) #2

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1170) (llvm/branches/ltoprof 1248)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
