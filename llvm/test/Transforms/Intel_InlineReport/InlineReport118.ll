; RUN: opt < %s -passes='require<profile-summary>,cgscc(inline)' -S -inline-report=0xe807 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<profile-summary>,cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Test that functions with attribute Hot are inlined while the 
; same function without attribute Hot will not be inlined.

; CHECK-CL-LABEL: define i32 @simpleFunction(i32 %a)
; CHECK-CL-LABEL: define i32 @HotFunction(i32 %a)
; CHECK-CL-LABEL: define i32 @HotFunction2(i32 %a)
; CHECK-CL-LABEL: define i32 @bar(i32 %a)
; CHECK-CL-NOT: tail call i32 @HotFunction(i32 5)
; CHECK-CL: tail call i32 @simpleFunction(i32 6)
; CHECK-CL-NOT: tail call i32 @HotFunction2(i32 5)

; CHECK-LABEL: COMPILE FUNC: simpleFunction
; CHECK-LABEL: COMPILE FUNC: HotFunction
; CHECK-LABEL: COMPILE FUNC: HotFunction2
; CHECK-LABEL: COMPILE FUNC: bar
; CHECK: INLINE: HotFunction{{.*}}Callee is hot
; CHECK-NOT: INLINE: simpleFunction
; CHECK: INLINE: HotFunction2{{.*}}Callee is hot

; CHECK-MD-LABEL: define i32 @simpleFunction(i32 %a)
; CHECK-MD-LABEL: define i32 @HotFunction(i32 %a)
; CHECK-MD-LABEL: define i32 @HotFunction2(i32 %a)
; CHECK-MD-LABEL: define i32 @bar(i32 %a)
; CHECK-MD-NOT: tail call i32 @HotFunction(i32 5)
; CHECK-MD: tail call i32 @simpleFunction(i32 6)
; CHECK-MD-NOT: tail call i32 @HotFunction2(i32 5)

@a = global i32 4

; This function should be greater than the default threshold (225).
; Function Attrs: nounwind readnone uwtable
define i32 @simpleFunction(i32 %a) #0 "function-inline-cost"="1000" {
entry:
  ret i32 %a
}

; This function should be smaller than the default threshold (225).
; Function Attrs: nounwind cold readnone uwtable
define i32 @HotFunction(i32 %a) #1 "function-inline-cost"="220" {
entry:
  ret i32 %a
}

; This function should be larger than the default threshold (225), but
; smaller than the hot threshold (325).
define i32 @HotFunction2(i32 %a) #1 "function-inline-cost"="320" {
entry:
  ret i32 %a
}

; Function Attrs: nounwind readnone uwtable
define i32 @bar(i32 %a) #0 {
entry:
  %0 = tail call i32 @HotFunction(i32 5)
  %1 = tail call i32 @simpleFunction(i32 6)
  %2 = tail call i32 @HotFunction2(i32 5)
  %3 = add i32 %0, %1
  %add = add i32 %2, %3
  ret i32 %add
}

declare void @extern()
attributes #0 = { nounwind readnone uwtable }
attributes #1 = { nounwind hot readnone uwtable }
