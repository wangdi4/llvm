; Test that generic cloning based on the if-switch heuristic occurred.

; REQUIRES: asserts
; RUN: opt < %s -ip-gen-cloning-enable-morphology -debug-only=ipcloning -ip-cloning -ip-cloning-after-inl -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -S 2>&1 | FileCheck %s
; RUN: opt < %s -ip-gen-cloning-enable-morphology -debug-only=ipcloning -passes='module(post-inline-ip-cloning)' -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -S 2>&1 | FileCheck %s

; CHECK: Enter IP cloning: (After inlining)
; CHECK: Cloning Analysis for:  foo
; CHECK: Selected generic cloning
; CHECK: Pending FORMAL_0
; CHECK: IFCount 1 <- 1
; CHECK: SwitchCount 0 <- 0
; CHECK: Pending FORMAL_1
; CHECK: IFCount 2 <- 1
; CHECK: SwitchCount 0 <- 0
; CHECK: Pending FORMAL_2
; CHECK: IFCount 2 <- 0
; CHECK: SwitchCount 1 <- 1
; CHECK: Selecting all Pending FORMALs

; CHECK: define dso_local i32 @main
; CHECK: call i32 @foo.2
; CHECK: call i32 @foo.1
; CHECK: define internal i32 @foo.1
; CHECK: define internal i32 @foo.2

define dso_local i32 @main() #0 {
entry:
  %call = call i32 @foo(i32 2, i32 3, i32 4)
  %call1 = call i32 @foo(i32 3, i32 4, i32 5)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

define internal i32 @foo(i32 %arg1, i32 %arg2, i32 %arg3) #0 {
entry:
  %retval = alloca i32, align 4
  %cmp = icmp sgt i32 %arg1, 5
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 5, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %arg2, 4
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  store i32 4, i32* %retval, align 4
  br label %return

if.end3:                                          ; preds = %if.end
  switch i32 %arg3, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
    i32 3, label %sw.bb
  ]

sw.bb:                                            ; preds = %if.end3, %if.end3, %if.end3
  store i32 3, i32* %retval, align 4
  br label %return

sw.epilog:                                        ; preds = %if.end3
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %sw.epilog, %sw.bb, %if.then2, %if.then
  %t3 = load i32, i32* %retval, align 4
  ret i32 %t3
}
