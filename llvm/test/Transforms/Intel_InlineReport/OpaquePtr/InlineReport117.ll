; RUN: opt -opaque-pointers < %s -passes='require<profile-summary>,cgscc(inline)' -S -inline-report=0xe807 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<profile-summary>,cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Test that functions with attribute Cold are not inlined while the
; same function without attribute Cold will be inlined.

; CHECK-CL-LABEL: define i32 @simpleFunction(i32 %a)
; CHECK-CL-LABEL: define i32 @ColdFunction(i32 %a)
; CHECK-CL-LABEL: define i32 @ColdFunction2(i32 %a)
; CHECK-CL-LABEL: define i32 @bar(i32 %a)
; CHECK-CL: tail call i32 @ColdFunction(i32 5)
; CHECK-CL: tail call i32 @ColdFunction2(i32 5)
; CHECK-CL-NOT: tail call i32 @simpleFunction(i32 6)

; CHECK-LABEL: COMPILE FUNC: simpleFunction
; CHECK-LABEL: COMPILE FUNC: ColdFunction
; CHECK-LABEL: COMPILE FUNC: ColdFunction2
; CHECK-LABEL: COMPILE FUNC: bar
; CHECK: ColdFunction{{.*}}Callee is cold
; CHECK: INLINE: simpleFunction
; CHECK: ColdFunction2{{.*}}Callee is cold

; CHECK-MD-LABEL: define i32 @simpleFunction(i32 %a)
; CHECK-MD-LABEL: define i32 @ColdFunction(i32 %a)
; CHECK-MD-LABEL: define i32 @ColdFunction2(i32 %a)
; CHECK-MD-LABEL: define i32 @bar(i32 %a)
; CHECK-MD: tail call i32 @ColdFunction(i32 5)
; CHECK-MD: tail call i32 @ColdFunction2(i32 5)
; CHECK-MD-NOT: tail call i32 @simpleFunction(i32 6)

@a = global i32 4

; Function Attrs: nounwind readnone uwtable
define i32 @simpleFunction(i32 %a) #0 {
entry:
  ret i32 %a
}

; Function Attrs: cold nounwind readnone uwtable
define i32 @ColdFunction(i32 %a) #1 {
entry:
  ret i32 %a
}

; Function Attrs: cold nounwind readnone uwtable
define i32 @ColdFunction2(i32 %a) #2 {
entry:
  ret i32 %a
}

; Function Attrs: nounwind readnone uwtable
define i32 @bar(i32 %a) #3 {
entry:
  %i = tail call i32 @ColdFunction(i32 5)
  %i1 = tail call i32 @simpleFunction(i32 6)
  %i2 = tail call i32 @ColdFunction2(i32 5)
  %i3 = add i32 %i, %i1
  %add = add i32 %i2, %i3
  ret i32 %add
}

declare void @extern()

attributes #0 = { nounwind readnone uwtable "function-inline-cost"="80" }
attributes #1 = { cold nounwind readnone uwtable "function-inline-cost"="55" }
attributes #2 = { cold nounwind readnone uwtable "function-inline-cost"="250" }
attributes #3 = { nounwind readnone uwtable }
