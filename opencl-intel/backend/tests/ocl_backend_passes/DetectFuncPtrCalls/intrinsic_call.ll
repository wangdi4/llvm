; RUN: opt < %s -analyze -detectfuncptrcall  | FileCheck %s

declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) nounwind
define i32 @test0(i8* %P) {
  
; CHECK: Printing analysis 'Detect Function Pointer Calls'
; CHECK: DetectFuncPtrCalls: not found function pointer calls.

  call void @llvm.memset.p0i8.i32(i8* %P, i8 0, i32 42, i32 1, i1 false)
   
  ret i32 0
  
}
