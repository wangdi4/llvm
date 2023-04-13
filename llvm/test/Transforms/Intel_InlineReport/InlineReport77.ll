; RUN: opt -opaque-pointers < %s -passes='partial-inliner,print<inline-report>' -skip-partial-inlining-cost-analysis -inline-report=0xe807 -disable-output 2>&1 | FileCheck %s

; Check that partial inlining is recorded in the classic inlining report.

; CHECK: DEAD STATIC FUNC: inlinedFunc.1
; CHECK: COMPILE FUNC: inlinedFunc
; CHECK: COMPILE FUNC: dummyCaller
; CHECK: INLINE: inlinedFunc.1{{.*}}Preferred for partial inlining
; CHECK: COMPILE FUNC: inlinedFunc.1.if.then

define internal i32 @inlinedFunc(i1 %cond, ptr align 4 %align.val) {
entry:
  br i1 %cond, label %if.then, label %return
if.then:
  ; Dummy store to have more than 0 uses
  store i32 10, ptr %align.val, align 4
  br label %return
return:             ; preds = %entry
  ret i32 0
}

define internal i32 @dummyCaller(i1 %cond, ptr align 2 %align.val) {
entry:
  %val = call i32 @inlinedFunc(i1 %cond, ptr %align.val)
  ret i32 %val
}

