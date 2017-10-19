; RUN: llc -mtriple=csa < %s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @hash(i8* noalias %arr, i64 %N) #0 {
entry:
  br label %loop

loop:
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %loop ]
  %hash = phi i64 [ 0, %entry ], [ %sum.next, %loop ]
  %addr = getelementptr inbounds i8, i8* %arr, i64 %indvar
  %val = load i8, i8* %addr, align 8
  %val.zext = zext i8 %val to i64
  %hash.add = mul i64 %hash, 37
  %sum.next = add i64 %val.zext, %hash.add
  %indvar.next = add nuw i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %N
  br i1 %exitcond, label %exit, label %loop
; CHECK: mov0 %[[INORD:[a-z0-9_]+]], 0
; CHECK: mov64 %[[LEN:[a-z0-9_]+]], %r3
; CHECK: mov64 %[[ADDR:[a-z0-9_]+]], %r2
; CHECK: sld8 %[[VAL:[a-z0-9_]+]], %[[ADDR]], %[[LEN]], 1, %[[OUTORD:[a-z0-9_]+]], %[[INORD]], MEMLEVEL_T0
; CHECK: mov0 %r1, %[[OUTORD]]

exit:
  ret i64 %sum.next
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
