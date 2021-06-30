; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
;
; Test passes run under -enable-dtrans in the compile step in the new pass manager
;
; RUN: opt -disable-output -disable-verify -enable-npm-dtrans \
; RUN:   -debug-pass-manager -passes='lto-pre-link<O0>' %s 2>&1 | FileCheck %s \
; RUN:   --check-prefixes=CHECK-NEWPM-O0
; RUN: opt -disable-output -disable-verify -enable-npm-dtrans \
; RUN:   -debug-pass-manager -passes='lto-pre-link<O1>' %s 2>&1 | FileCheck %s \
; RUN:   --check-prefixes=CHECK-NEWPM-O123
; RUN: opt -disable-output -disable-verify -enable-npm-dtrans \
; RUN:   -debug-pass-manager -passes='lto-pre-link<O3>' %s 2>&1 | FileCheck %s \
; RUN:   --check-prefixes=CHECK-NEWPM-O123
; RUN: opt -disable-output -disable-verify -enable-npm-dtrans \
; RUN:   -debug-pass-manager -passes='lto-pre-link<O3>' %s 2>&1 | FileCheck %s \
; RUN:   --check-prefixes=CHECK-NEWPM-O123

; These passes should not be enabled at -O0

; CHECK-NEWPM-O0-NOT: Running pass: FunctionRecognizerPass on foo
; CHECK-NEWPM-O0-NOT: Running pass: FunctionRecognizerPass on main

; These passes should be enabled at optimization levels over -O0

; CHECK-NEWPM-O123: Running pass: FunctionRecognizerPass on foo
; CHECK-NEWPM-O123: Running pass: FunctionRecognizerPass on main

declare void @bar() local_unnamed_addr

; Function Attrs: noinline uwtable
define void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}

define i32 @main() {
  call void @foo(i32 1)
  ret i32 0
}

attributes #0 = { noinline uwtable }
; end INTEL_FEATURE_SW_ADVANCED
