; REQUIRES: asserts
;
; Test passes run under -enable-dtrans in the compile step in the old pass manager
;

; RUN: opt -enable-new-pm=0 -O0 -disable-output -disable-verify -enable-dtrans -debug-pass=Executions -prepare-for-lto %s 2>&1 | FileCheck %s --check-prefixes=CHECK-OLDPM-O0
; RUN: opt -enable-new-pm=0 -O1 -disable-output -disable-verify -enable-dtrans -debug-pass=Executions -prepare-for-lto %s 2>&1 | FileCheck %s --check-prefixes=CHECK-OLDPM-O123
; RUN: opt -enable-new-pm=0 -O2 -disable-output -disable-verify -enable-dtrans -debug-pass=Executions -prepare-for-lto %s 2>&1 | FileCheck %s --check-prefixes=CHECK-OLDPM-O123
; RUN: opt -enable-new-pm=0 -O3 -disable-output -disable-verify -enable-dtrans -debug-pass=Executions -prepare-for-lto %s 2>&1 | FileCheck %s --check-prefixes=CHECK-OLDPM-O123

; These passes should not be enabled at -O0

; CHECK-OLDPM-OO: FunctionPass Manager
; CHECK-OLDPM-O0-NOT: Function Recognizer
; CHECK-OLDPM-O0-NOT: Executing Pass 'Function Recognizer' on Function 'foo'
; CHECK-OLDPM-O0-NOT: Freeing Pass 'Function Recognizer' on Function 'foo'
; CHECK-OLDPM-O0-NOT: Executing Pass 'Function Recognizer' on Function 'main'
; CHECK-OLDPM-O0-NOT: Freeing Pass 'Function Recognizer' on Function 'main'

; But these passes should be enabled at optimization levels over -O0

; CHECK-OLDPM-O123: FunctionPass Manager
; CHECK-OLDPM-O123: Function Recognizer
; CHECK-OLDPM-O123: Executing Pass 'Function Recognizer' on Function 'foo'
; CHECK-OLDPM-O123: Freeing Pass 'Function Recognizer' on Function 'foo'
; CHECK-OLDPM-O123: Executing Pass 'Function Recognizer' on Function 'main'
; CHECK-OLDPM-O123: Freeing Pass 'Function Recognizer' on Function 'main'

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
