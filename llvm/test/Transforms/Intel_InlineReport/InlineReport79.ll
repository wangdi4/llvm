; RUN: opt < %s -passes='partial-inliner,print<inline-report>' -skip-partial-inlining-cost-analysis -inline-report=0xe807 -disable-output 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-CL %s
; RUN: opt < %s -passes='inlinereportsetup,partial-inliner,inlinereportemitter' -skip-partial-inlining-cost-analysis -inline-report=0xe886 -disable-output 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-MD %s

; Check that partial inlining is recorded in the inlining report and that the
; call to foo appears both in inlinedFunc and dummyCaller after the partial
; inlining.

; CHECK-CL: DEAD STATIC FUNC: inlinedFunc.1
; CHECK: COMPILE FUNC: inlinedFunc
; CHECK-CL: foo{{.*}}Newly created callsite
; CHECK-MD: foo{{.*}}Not tested for inlining
; CHECK: COMPILE FUNC: dummyCaller
; CHECK: INLINE: inlinedFunc.1{{.*}}Preferred for partial inlining
; CHECK-CL: foo{{.*}}Newly created callsite
; CHECK-MD: foo{{.*}}Not tested for inlining
; CHECK-MD: DEAD STATIC FUNC: inlinedFunc.1
; CHECK: COMPILE FUNC: inlinedFunc.1.if.then

define internal i32 @inlinedFunc(i1 %cond, ptr align 4 %align.val) {
entry:
  %t1 = call i32 @foo()
  br i1 %cond, label %if.then, label %return
if.then:
  ; Dummy store to have more than 0 uses
  store i32 10, ptr %align.val, align 4
  br label %return
return:             ; preds = %entry
  ret i32 %t1
}

define internal i32 @dummyCaller(i1 %cond, ptr align 2 %align.val) {
entry:
  %val = call i32 @inlinedFunc(i1 %cond, ptr %align.val)
  ret i32 %val
}

define internal i32 @foo() {
  ret i32 5
}
