; RUN: opt -opaque-pointers < %s -passes=partial-inliner -skip-partial-inlining-cost-analysis -disable-output
; This testcase tests that an llvm.assume intrinsic inside the splinter
; function will not cause the compiler to assert because
;   OutlinedFunctionCost < Cloner.OutlinedRegionCost
; The issue is that a llvm.assume instruction will not be replicated in
; the code extracted for the splinter function, and so should not be counted
; in the cost.

; Function Attrs: inaccessiblememonly nocallback nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #0

define internal i32 @inlinedFunc(i1 %cond, ptr align 4 %align.val) {
entry:
  br i1 %cond, label %if.then, label %return

if.then:                                          ; preds = %entry
  call void @llvm.assume(i1 true)
  store i32 10, ptr %align.val, align 4
  br label %return

return:                                           ; preds = %if.then, %entry
  ret i32 0
}

define internal i32 @dummyCaller(i1 %cond, ptr align 2 %align.val) {
entry:
  %val = call i32 @inlinedFunc(i1 %cond, ptr %align.val)
  ret i32 %val
}

attributes #0 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
