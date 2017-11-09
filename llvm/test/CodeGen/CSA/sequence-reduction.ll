; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define double @copy(double* noalias %arr, i64 %N) #0 {
entry:
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %sum.loop = phi double [ 0.0, %entry ], [ %sum, %loop ]
  %addr.arr = getelementptr inbounds double, double* %arr, i64 %indvar
  %val = load double, double* %addr.arr, align 8
  %sum = fadd double %sum.loop, %val
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp slt i64 %indvar.next, %N
  br i1 %exitcond, label %loop, label %exit
; CHECK: mov0 %[[INORD:[a-z0-9_]+]], 0
; CHECK: mov64 %[[LEN:[a-z0-9_]+]], %r3
; CHECK: mov64 %[[SRC:[a-z0-9_]+]], %r2
; CHECK: mov64 %[[INITA:[a-z0-9_]+]], 0
; CHECK: mov64 %[[INITB:[a-z0-9_]+]], 0
; CHECK: sredaddf64 %[[REDUCE:[a-z0-9_]+]], %ign, %[[INITB]], %[[VAL:[a-z0-9_]+]], 
; CHECK: sld64 %[[VAL]], %[[SRC]], %[[LEN]], 1, %[[OUTORD:[a-z0-9_]+]], %[[INORD]], MEMLEVEL_T0
; CHECK: mov0 %r1, %[[OUTORD]]

exit:
  ret double %sum
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
