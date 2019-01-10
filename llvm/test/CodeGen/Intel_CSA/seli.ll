; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @seli(i32 %i) #0 {
; CSA_CHECK-LABEL: seli
; CSA_CHECK: sll32

entry:
  %tobool = icmp ne i32 %i, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %add = add nsw i32 %i, 1
  br label %return

if.end:                                           ; preds = %entry
  %mul = mul nsw i32 %i, 2
  br label %return

return:                                           ; preds = %if.end, %if.then
  %0 = phi i32 [ %add, %if.then ], [ %mul, %if.end]
  ret i32 %0
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
