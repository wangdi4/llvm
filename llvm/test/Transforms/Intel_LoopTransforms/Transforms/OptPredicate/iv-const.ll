; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -disable-output < %s 2>&1 | FileCheck %s

; HIR:
;           BEGIN REGION { }
; <21>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <5>             |   if (i1 + -100 <= 100)
; <5>             |   {
; <10>            |      (%q)[i1] = i1;
; <5>             |   }
; <14>            |   (%p)[i1] = i1;
; <21>            + END LOOP
;           END REGION

; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i1 = 0, (-1 + (-1 * smax(-201, (-1 * %n)))), 1   <DO_LOOP>
; CHECK:          |   (%q)[i1] = i1;
; CHECK:          |   (%p)[i1] = i1;
; CHECK:          + END LOOP
;
;
; CHECK:          + DO i1 = 0, %n + -202, 1   <DO_LOOP>
; CHECK:          |   (%p)[i1 + 201] = i1 + 201;
; CHECK:          + END LOOP
; CHECK:    END REGION

;Module Before HIR; ModuleID = 'iv-const.c'
source_filename = "iv-const.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture %q, i64 %n, i64 %d) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i64 %n, 0
  br i1 %cmp14, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %j.015 = phi i64 [ %inc, %if.end ], [ 0, %for.body.preheader ]
  %sub = add nsw i64 %j.015, -100
  %cmp1 = icmp sgt i64 %sub, 100
  %conv = trunc i64 %j.015 to i32
  br i1 %cmp1, label %if.end, label %if.else

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %q, i64 %j.015
  store i32 %conv, i32* %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %for.body, %if.else
  %arrayidx5.sink = getelementptr inbounds i32, i32* %p, i64 %j.015
  store i32 %conv, i32* %arrayidx5.sink, align 4
  %inc = add nuw nsw i64 %j.015, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


