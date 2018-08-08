; RUN: llc -mtriple=csa < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @hash(i64* noalias %arr, i64 %N) #0 {
entry:
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %addr = getelementptr inbounds i64, i64* %arr, i64 %indvar
  store i64 %indvar, i64* %addr, align 8
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop
; CHECK: .result .lic .i1 %[[OUTORD:[a-z0-9_]+]]
; CHECK: .param .lic .i1 %[[INORD:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[ADDR:[a-z0-9_]+]]
; CHECK: .param .lic .i64 %[[LEN:[a-z0-9_]+]]
; CHECK: cmpne64 %[[OT:[a-z0-9_]+]], 0, %[[LEN]]
; CHECK: merge64 %[[SAFELEN:[a-z0-9_]+]], %[[OT]], 1, %[[LEN]]
; CHECK: sst64 %[[ADDR]], %[[SAFELEN]], 1, %[[VAL:[a-z0-9_]+]], %[[OUTORD:[a-z0-9_]+]], %[[INORD]], MEMLEVEL_T0

exit:
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
