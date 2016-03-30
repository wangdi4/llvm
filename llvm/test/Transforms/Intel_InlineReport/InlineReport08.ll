; RUN: opt -inline -inline-report=33 -inline-threshold=50 -inlinehint-threshold=100 -inlinecold-threshold=25 -inlineoptsize-threshold=10 < %s -S 2>&1 | FileCheck %s

; Generated with clang -c -S -emit-llvm sm1.c 

; CHECK: Begin 
; CHECK-NEXT: Option Values:
; CHECK-NEXT: inline-threshold: 50
; CHECK-NEXT: inlinehint-threshold: 100
; CHECK-NEXT: inlinecold-threshold: 25
; CHECK-NEXT: inlineoptsize-threshold: 10 

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: DEAD STATIC FUNC: L foo

; CHECK: COMPILE FUNC: A main
; CHECK-NEXT: INLINE: L foo

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @foo()
  ret i32 %call
}


; Function Attrs: alwaysinline nounwind uwtable
define internal i32 @foo() #0 {
entry:
  ret i32 5
}

; CHECK: End Inlining Report

attributes #0 = { alwaysinline nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 831) (llvm/branches/ltoprof 938)"}
