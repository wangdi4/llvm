; This test checks that whole program assume won't internalize
; @__dso_handle since it is considered as a linker added symbol.
; The test case is marked as 'not' because the linker will throw
; a linking failure due to undefined reference to __dso_handle.

; RUN: llvm-as %s -o %t.bc
; RUN: not %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-whole-program-assume \
; RUN:    -plugin-opt=-print-after-all  \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that __dso_handle is external

; CHECK: @__dso_handle = external hidden global i8

; Check that __dso_handle wasn't internalized

; CHECK: @__dso_handle = external hidden global i8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__dso_handle = external hidden global i8

declare i32 @foo(i8*)

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @foo(i8* nonnull @__dso_handle)
  ret i32 %call1
}
