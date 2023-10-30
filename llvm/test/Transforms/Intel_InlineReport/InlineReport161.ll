; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f847-SIZE %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f8c6 %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=size -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f847-SIZE %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=size -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f8c6 %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=cost -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f847-OTHER %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=cost -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f8c6 %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=cost-benefit -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f847-OTHER %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=cost-benefit -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f8c6 %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=ml -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f847-OTHER %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=ml -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK-f8c6 %s

; Basic inlining tests for the module inliner.


; CHECK-f847-SIZE: COMPILE FUNC: callee1
; CHECK-f847-SIZE: COMPILE FUNC: caller
; CHECK-f847-SIZE: INLINE: callee1 {{.*}}Callee is single basic block
; CHECK-f847-SIZE: callee2 {{.*}}Callee has noinline attribute
; CHECK-f847-SIZE: COMPILE FUNC: callee2

; CHECK-f847-OTHER: COMPILE FUNC: callee1
; CHECK-f847-OTHER: COMPILE FUNC: caller
; CHECK-f847-OTHER: INLINE: callee1 {{.*}}Callee is single basic block
; CHECK-f847-OTHER: callee2 {{.*}}Callee has noinline attribute
; CHECK-f847-OTHER: COMPILE FUNC: callee2

; CHECK-f8c6: COMPILE FUNC: callee1
; CHECK-f8c6: COMPILE FUNC: callee2
; CHECK-f8c6: COMPILE FUNC: caller
; CHECK-f8c6: INLINE: callee1 {{.*}}Callee is single basic block
; CHECK-f8c6: callee2 {{.*}}Callee has noinline attribute

define i32 @callee1(i32 %a) {
entry:
  %add = add nsw i32 %a, 1
  ret i32 %add
}

define i32 @callee2(i32 %a) #0 {
entry:
  %add = add nsw i32 %a, 1
  ret i32 %add
}

define i32 @caller(i32 %a) {
entry:
  %call1 = call i32 @callee1(i32 %a)
  %call2 = call i32 @callee2(i32 %a)
  %call = add nsw i32 %call1, %call2
  ret i32 %call
}

attributes #0 = { noinline }
