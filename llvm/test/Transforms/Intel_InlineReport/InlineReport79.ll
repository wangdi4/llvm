; RUN: opt < %s -passes=partial-inliner -skip-partial-inlining-cost-analysis -inline-report=0xe807 -force-print-inline-report-after-partial-inline -disable-output 2>&1 | FileCheck %s

; Check that partial inlining is recorded in the classic inlining report and
; that the call to foo appears both in inlinedFunc and dummyCaller after the
; partial inlining.

; CHECK: DEAD STATIC FUNC: inlinedFunc.1
; CHECK: COMPILE FUNC: inlinedFunc
; CHECK: foo{{.*}}Newly created callsite
; CHECK: COMPILE FUNC: dummyCaller
; CHECK: INLINE: inlinedFunc.1{{.*}}Preferred for partial inlining
; CHECK: foo{{.*}}Newly created callsite
; CHECK: COMPILE FUNC: inlinedFunc.1.if.then

define internal i32 @inlinedFunc(i1 %cond, i32* align 4 %align.val) {
entry:
  %t1 = call i32 @foo()
  br i1 %cond, label %if.then, label %return
if.then:
  ; Dummy store to have more than 0 uses
  store i32 10, i32* %align.val, align 4
  br label %return
return:             ; preds = %entry
  ret i32 %t1
}

define internal i32 @dummyCaller(i1 %cond, i32* align 2 %align.val) {
entry:
  %val = call i32 @inlinedFunc(i1 %cond, i32* %align.val)
  ret i32 %val
}

define internal i32 @foo() {
  ret i32 5
}
