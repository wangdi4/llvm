; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe801 -inlinehint-threshold=100 -inlinecold-threshold=25 -inlineoptsize-threshold=10 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-INLREP
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe880 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe880 -S -inlinehint-threshold=100 -inlinecold-threshold=25 -inlineoptsize-threshold=10 | opt -passes='inlinereportemitter' -inline-report=0xe880 -inlinehint-threshold=100 -inlinecold-threshold=25 -inlineoptsize-threshold=10 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD-INLREP

; Generated with clang -c -S -emit-llvm sm1.c
; Inline inlineoptsize-threshold will not be overridden and will print as 10.

; CHECK: Begin
; CHECK-NEXT: Option Values:
; CHECK-NEXT: inline-threshold: 225
; CHECK-NEXT: inlinehint-threshold: 100
; CHECK-NEXT: inlinecold-threshold: 25
; CHECK-NEXT: inlineoptsize-threshold: 10

; CHECK-INLREP: DEAD STATIC FUNC: foo

; CHECK: COMPILE FUNC: main
; CHECK-NEXT: INLINE: foo

; CHECK-MD-INLREP: DEAD STATIC FUNC: foo

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval
  %call = call i32 @foo()
  ret i32 %call
}


; Function Attrs: alwaysinline nounwind uwtable
define internal i32 @foo() {
entry:
  ret i32 5
}

; CHECK: End Inlining Report

