; Test that the intel_advancedfastcall pass creates a version of an
; address taken function that can be converted to use the 'fastcc'
; calling convention.

; RUN: opt < %s -S -intel-advancedfastcall -intel-advancedfastcall-require-profile=false | FileCheck %s
; RUN: opt < %s -S -passes=intel-advancedfastcall -intel-advancedfastcall-require-profile=false | FileCheck %s

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@g_fptr1 = internal global void (i32, i32)* @test1
@g_fptr2 = internal global i32 (i32, i32)* zeroinitializer
@g_fptr3 = internal global i32 (i32, ...)* @test3
@g_fptr4 = internal global i32 (i32)* @test4

; Verify the address taken function was the wrapper function
; CHECK: @g_fptr1 = internal global void (i32, i32)* @test1
; CHECK: @g_fptr2 = internal global i32 (i32, i32)* null
; CHECK: @g_fptr3 = internal global i32 (i32, ...)* @test3
; CHECK: @g_fptr4 = internal global i32 (i32)* @test4

define internal void @test1(i32 %a1, i32 %a2) {
  ret void
}
; Verify the address taken function was changed into a wrapper function
; that calls the cloned function.
; CHECK-LABEL: define internal void @test1(i32 %0, i32 %1)
; CHECK: tail call void @test1.1(i32 %0, i32 %1)
; CHECK: ret void

; CHECK-LABEL: define internal void @test1.1(i32 %a1, i32 %a2) {
; CHECK: ret void


; Test with a function that returns a value
define internal i32 @test2(i32 %a1, i32 %a2) {
 ret i32 0;
}
; CHECK-LABEL: define internal i32 @test2(i32 %0, i32 %1)
; CHECK: %3 = tail call i32 @test2.2(i32 %0, i32 %1)
; CHECK: ret i32 %3

; CHECK-LABEL: define internal i32 @test2.2(i32 %a1, i32 %a2)
; CHECK: ret i32 0


; vararg call should not be converted
define internal i32 @test3(i32 %a1, ...) {
  ret i32 0
}
; CHECK-LABEL: define internal i32 @test3(i32 %a1, ...)
; CHECK: ret i32 0


; For now, only 'internal' definitions are changed. This could
; be relaxed in the future.
define i32 @test4(i32 %a1) {
  ret i32 0
}
; CHECK-LABEL: i32 @test4(i32 %a1)
; CHECK: ret i32 0

; Check that the direct calls are changed to the new function.
define i32 @main() {
  store i32 (i32, i32)* @test2, i32 (i32, i32)** @g_fptr2

  call void @test1(i32 0, i32 8)
  %call2 = call i32 @test2(i32 0, i32 8)
  %call3 = call i32 (i32, ...) @test3(i32 3, i32 2, i32 1, i32 0)
  %call4 = call i32 @test4(i32 0)
  ret i32 0;
}

; Verify the clones were called for test1 and test2.
; CHECK-LABEL: define i32 @main
; CHECK: call void @test1.1(i32 0, i32 8)
; CHECK: call i32 @test2.2(i32 0, i32 8)
; CHECK: call i32 (i32, ...) @test3(i32 3, i32 2, i32 1, i32 0)
; CHECK: call i32 @test4(i32 0)
