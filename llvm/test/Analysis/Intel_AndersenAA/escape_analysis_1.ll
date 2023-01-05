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
@foo.fooBuf = internal unnamed_addr global i32* null, align 8
@foo.local_fooBuf = internal global [2048 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %fooPtr, i32 %aconst) #0 {
entry:
  %.b = load i1, i1* @foo.init, align 1
  br i1 %.b, label %entry.if.end_crit_edge, label %if.then

entry.if.end_crit_edge:                           ; preds = %entry
  %.pre = load i32*, i32** @foo.fooBuf, align 8
  br label %if.end

if.then:                                          ; preds = %entry
  store i32* getelementptr inbounds ([2048 x i32], [2048 x i32]* @foo.local_fooBuf, i64 0, i64 0), i32** @foo.fooBuf, align 8
  store i1 true, i1* @foo.init, align 1
  br label %if.end

if.end:                                           ; preds = %entry.if.end_crit_edge, %if.then
  %0 = phi i32* [ %.pre, %entry.if.end_crit_edge ], [ getelementptr inbounds ([2048 x i32], [2048 x i32]* @foo.local_fooBuf, i64 0, i64 0), %if.then ]
  %div = sdiv i32 %aconst, 2
  %idxprom = sext i32 %div to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  %cmp1.4 = icmp sgt i32 %aconst, 0
  br i1 %cmp1.4, label %for.inc.lr.ph, label %for.end

for.inc.lr.ph:                                    ; preds = %if.end
  %1 = add i32 %aconst, -1
  %xtraiter = and i32 %aconst, 3
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.inc.lr.ph.split, label %for.inc.prol

for.inc.prol:                                     ; preds = %for.inc.prol, %for.inc.lr.ph
  %indvars.iv.prol = phi i64 [ 0, %for.inc.lr.ph ], [ %indvars.iv.next.prol, %for.inc.prol ]
  %sum.05.prol = phi i32 [ 0, %for.inc.lr.ph ], [ %add.prol, %for.inc.prol ]
  %prol.iter = phi i32 [ %xtraiter, %for.inc.lr.ph ], [ %prol.iter.sub, %for.inc.prol ]
  %arrayidx3.prol = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.prol
  %2 = trunc i64 %indvars.iv.prol to i32
  store i32 %2, i32* %arrayidx3.prol, align 4
  %3 = load i32, i32* %arrayidx, align 4
  %add.prol = add nsw i32 %3, %sum.05.prol
  %indvars.iv.next.prol = add nuw nsw i64 %indvars.iv.prol, 1
  %prol.iter.sub = add i32 %prol.iter, -1
  %prol.iter.cmp = icmp eq i32 %prol.iter.sub, 0
  br i1 %prol.iter.cmp, label %for.inc.lr.ph.split, label %for.inc.prol, !llvm.loop !1

for.inc.lr.ph.split:                              ; preds = %for.inc.prol, %for.inc.lr.ph
  %indvars.iv.unr = phi i64 [ 0, %for.inc.lr.ph ], [ %indvars.iv.next.prol, %for.inc.prol ]
  %sum.05.unr = phi i32 [ 0, %for.inc.lr.ph ], [ %add.prol, %for.inc.prol ]
  %add.lcssa.unr = phi i32 [ undef, %for.inc.lr.ph ], [ %add.prol, %for.inc.prol ]
  %4 = icmp ult i32 %1, 3
  br i1 %4, label %for.end, label %for.inc

for.inc:                                          ; preds = %for.inc.lr.ph.split, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next.3, %for.inc ], [ %indvars.iv.unr, %for.inc.lr.ph.split ]
  %sum.05 = phi i32 [ %add.3, %for.inc ], [ %sum.05.unr, %for.inc.lr.ph.split ]
  %arrayidx3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv
  %5 = trunc i64 %indvars.iv to i32
  store i32 %5, i32* %arrayidx3, align 4
  %6 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %6, %sum.05
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %arrayidx3.1 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next
  store i32 %lftr.wideiv, i32* %arrayidx3.1, align 4
  %7 = load i32, i32* %arrayidx, align 4
  %add.1 = add nsw i32 %7, %add
  %indvars.iv.next.1 = add nsw i64 %indvars.iv, 2
  %lftr.wideiv.1 = trunc i64 %indvars.iv.next.1 to i32
  %arrayidx3.2 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.1
  store i32 %lftr.wideiv.1, i32* %arrayidx3.2, align 4
  %8 = load i32, i32* %arrayidx, align 4
  %add.2 = add nsw i32 %8, %add.1
  %indvars.iv.next.2 = add nsw i64 %indvars.iv, 3
  %lftr.wideiv.2 = trunc i64 %indvars.iv.next.2 to i32
  %arrayidx3.3 = getelementptr inbounds i32, i32* %fooPtr, i64 %indvars.iv.next.2
  store i32 %lftr.wideiv.2, i32* %arrayidx3.3, align 4
  %9 = load i32, i32* %arrayidx, align 4
  %add.3 = add nsw i32 %9, %add.2
  %indvars.iv.next.3 = add nsw i64 %indvars.iv, 4
  %lftr.wideiv.3 = trunc i64 %indvars.iv.next.3 to i32
  %exitcond.3 = icmp eq i32 %lftr.wideiv.3, %aconst
  br i1 %exitcond.3, label %for.end, label %for.inc

for.end:                                          ; preds = %for.inc.lr.ph.split, %for.inc, %if.end
  %sum.0.lcssa = phi i32 [ 0, %if.end ], [ %add.lcssa.unr, %for.inc.lr.ph.split ], [ %add.3, %for.inc ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1310) (llvm/branches/ltoprof 1384)"}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.unroll.disable"}
