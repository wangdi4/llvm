; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=size -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=size -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=cost -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=cost -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=cost-benefit -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=cost-benefit -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s
; RUN: opt -passes='module-inline,print<inline-report>' -inline-report=0xf847 -inline-priority-mode=ml -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module-inline,inlinereportemitter' -inline-report=0xf8c6 -inline-priority-mode=ml -disable-output < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s

; Basic inlining tests for the module inliner.

; CHECK: COMPILE FUNC: callee1
; CHECK-MD: COMPILE FUNC: callee2
; CHECK: COMPILE FUNC: caller
; CHECK: INLINE: callee1 {{.*}}Callee is single basic block
; CHECK: callee2 {{.*}}Callee has noinline attribute
; CHECK-CL: COMPILE FUNC: callee2

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
