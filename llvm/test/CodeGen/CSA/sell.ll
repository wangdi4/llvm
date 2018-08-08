; RUN: llvm-link %CSABASE_ROOT/libcsa/libcsac.bc %s -o %t
; RUN: llc -mtriple=csa < %t | FileCheck %s --check-prefix=CSA_CHECK
; RUN: rm %t

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @sell(i64 %i) #0 {
; CSA_CHECK-LABEL: sell
; CSA_CHECK: sll64

entry:
  %retval = alloca i64, align 8
  %i.addr = alloca i64, align 8
  store i64 %i, i64* %i.addr, align 8
  %0 = load i64, i64* %i.addr, align 8
  %tobool = icmp ne i64 %0, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load i64, i64* %i.addr, align 8
  %add = add nsw i64 %1, 1
  store i64 %add, i64* %retval
  br label %return

if.end:                                           ; preds = %entry
  %2 = load i64, i64* %i.addr, align 8
  %mul = mul nsw i64 %2, 2
  store i64 %mul, i64* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %3 = load i64, i64* %retval
  ret i64 %3
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
