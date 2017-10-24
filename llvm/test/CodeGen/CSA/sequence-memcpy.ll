; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @copy(double* noalias %dest, double* noalias %src, i64 %N) #0 {
entry:
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %addr.src = getelementptr inbounds double, double* %src, i64 %indvar
  %addr.dest = getelementptr inbounds double, double* %dest, i64 %indvar
  %val = load double, double* %addr.src, align 8
  store double %val, double* %addr.dest, align 8
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop
; CHECK: mov0 %[[INORD:[a-z0-9_]+]], 0
; CHECK: mov64 %[[LEN:[a-z0-9_]+]], %r4
; CHECK: mov64 %[[SRC:[a-z0-9_]+]], %r3
; CHECK: mov64 %[[DEST:[a-z0-9_]+]], %r2
; CHECK: sld64 %[[VAL:[a-z0-9_]+]], %[[SRC]], %[[LEN]], 1, %[[OUTORDL:[a-z0-9_]+]], %[[INORD]], MEMLEVEL_T0
; CHECK: sst64 %[[DEST]], %[[LEN]], 1, %[[VAL]], %[[OUTORDS:[a-z0-9_]+]], %[[INORD]], MEMLEVEL_T0
; CHECK: all0 %[[OUTORD:[a-z0-9_]+]], %[[OUTORDL]], %[[OUTORDS]], %ign, %ign
; CHECK: mov0 %r0, %[[OUTORD]]

exit:
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
