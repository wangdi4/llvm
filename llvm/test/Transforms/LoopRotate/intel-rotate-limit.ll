; RUN: opt -loop-rotate-max-complexity=1 -passes="loop-rotate" -S %s | FileCheck %s
; Simple check for the complexity limit in loop rotation.
; A real case would have a blocks*loops factor of > 1e9.

; Loop should not rotate if the limit is hit.
; CHECK: add {{.*}} %i
; CHECK-NEXT: br label

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable willreturn mustprogress
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc1, %for.body ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %inc1 = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !2
}

attributes #0 = { norecurse nounwind readnone uwtable willreturn mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
