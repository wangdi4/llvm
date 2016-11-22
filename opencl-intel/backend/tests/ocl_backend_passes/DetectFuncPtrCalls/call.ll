; RUN: opt < %s -analyze -detectfuncptrcall  | FileCheck %s

declare noalias i32* @_Znwj(i32 %x) nounwind

define i32 @foo() {
; CHECK: Printing analysis 'Detect Function Pointer Calls'
; CHECK: DetectFuncPtrCalls: not found function pointer calls.
  %A = call i32* @_Znwj(i32 4)
  %B = call i32* @_Znwj(i32 4)
  store i32 1, i32* %A
  store i32 2, i32* %B
  %C = load i32, i32* %A
  ret i32 %C
}
