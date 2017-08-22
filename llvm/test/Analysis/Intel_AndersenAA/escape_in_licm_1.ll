; It checks escape analysis helps LICM to hoist invariant load out of loop
; RUN: opt < %s  -anders-aa  -licm  -disable-output  -stats -S 2>&1 | grep "1 licm"
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.init = internal unnamed_addr global i1 false
@foo.fooBuf = internal unnamed_addr global [2 x [1024 x i32]]* null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %fooPtr, i32 %aconst, i32 %n) #0 {
entry:
  %.b = load i1, i1* @foo.init, align 1
  br i1 %.b, label %entry.if.end_crit_edge, label %if.then

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load [2 x [1024 x i32]]*, [2 x [1024 x i32]]** @foo.fooBuf, align 8
  br label %if.end

if.then:                                          ; preds = %entry
  store [2 x [1024 x i32]]* bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), [2 x [1024 x i32]]** @foo.fooBuf, align 8
  store i1 true, i1* @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %entry.if.end_crit_edge, %if.then
  %0 = phi [2 x [1024 x i32]]* [ %.pre, %entry.if.end_crit_edge ], [ bitcast ([2048 x i32]* @foo.local_fooBuf to [2 x [1024 x i32]]*), %if.then ]
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %idxprom1 = sext i32 %n to i64
  %arrayidx2 = getelementptr inbounds [2 x [1024 x i32]], [2 x [1024 x i32]]* %0, i64 0, i64 %idxprom1, i64 %idxprom
  %cmp3.4 = icmp sgt i32 %aconst, 0
  br i1 %cmp3.4, label %for.inc, label %for.end

for.inc:                                          ; preds = %if.end, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %if.end ]
  %sum.05 = phi i32 [ %add, %for.inc ], [ 0, %if.end ]
  %arrayidx5 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx5, align 4
  %2 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %2, %sum.05
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %aconst
  br i1 %exitcond, label %for.end, label %for.inc

for.end:                                          ; preds = %for.inc, %if.end
  %sum.0.lcssa = phi i32 [ 0, %if.end ], [ %add, %for.inc ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1485)"}
