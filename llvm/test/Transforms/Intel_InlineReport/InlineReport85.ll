; RUN: opt < %s -inline -argpromotion -inline-report=7 -S 2>&1 | FileCheck %s --check-prefix=CHECK-OLD
; RUN: opt < %s -passes='cgscc(inline),module(argpromotion)' -inline-report=7 -S 2>&1 | FileCheck %s --check-prefix=CHECK-NEW

; Check that the classic inlining report does not produce messages about
; a deleted callsite followed by a newly created callsite, due to argument
; promotion.

; CHECK-OLD: COMPILE FUNC: test
; CHECK-OLD: COMPILE FUNC: callercaller
; CHECK-OLD: caller{{.*}}Callee has noinline attribute
; CHECK-OLD-NOT: Newly created callsite
; CHECK-OLD-NOT: DELETE
; CHECK-OLD: COMPILE FUNC: caller
; CHECK-OLD: test{{.*}}Callee has noinline attribute
; CHECK-OLD-NOT: Newly created callsite
; CHECK-OLD-NOT: DELETE

; CHECK-NEW: COMPILE FUNC: callercaller
; CHECK-NEW: caller{{.*}}Callee has noinline attribute
; CHECK-NEW-NOT: Newly created callsite
; CHECK-NEW-NOT: DELETE
; CHECK-NEW: COMPILE FUNC: test
; CHECK-NEW: COMPILE FUNC: caller
; CHECK-NEW: test{{.*}}Callee has noinline attribute
; CHECK-NEW-NOT: Newly created callsite
; CHECK-NEW-NOT: DELETE

define internal i32 @test(i32* %X, i32* %Y) #0 {
  %A = load i32, i32* %X
  %B = load i32, i32* %Y
  %C = add i32 %A, %B
  ret i32 %C
}

define internal i32 @caller(i32* %B) #0 {
  %A = alloca i32
  store i32 1, i32* %A
  %C = call i32 @test(i32* %A, i32* %B)
  ret i32 %C
}

define i32 @callercaller() {
  %B = alloca i32
  store i32 2, i32* %B
  %X = call i32 @caller(i32* %B)
  ret i32 %X
}

attributes #0 = { noinline }
