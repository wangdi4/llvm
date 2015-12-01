; RUN: opt < %s  -anders-aa -domtree -loops -loop-simplify -lcssa -basicaa -aa -nonltoglobalopt  -S | grep alloca
; The compiler should replace the global variable @p with the local
; variable %p thus the store to @p can be eliminated.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@foo.p = internal unnamed_addr global i32* null, align 8

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32* %a) #0 {
entry:
  store i32* %a, i32** @foo.p, align 8, !tbaa !1
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = add i32 %n, -1
  %1 = zext i32 %0 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %incdec.ptr6 = phi i32* [ %a, %for.body.lr.ph ], [ %incdec.ptr, %for.body ]
  %i.05 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %2 = load i32, i32* %incdec.ptr6, align 4, !tbaa !5
  %add = add nsw i32 %2, %i.05
  store i32 %add, i32* %incdec.ptr6, align 4, !tbaa !5
  %incdec.ptr = getelementptr inbounds i32, i32* %incdec.ptr6, i64 1
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %3 = add nuw nsw i64 %1, 1
  %scevgep = getelementptr i32, i32* %a, i64 %3
  store i32* %scevgep, i32** @foo.p, align 8, !tbaa !1
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1485)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"any pointer", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
