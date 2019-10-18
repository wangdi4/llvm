; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-rec-callsites=4 -debug-only=ipcloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-rec-callsites=4 -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that foo is not selected for generic cloning of a recursive routine
; because it does not have enough callsites.

; CHECK: Enter IP cloning: (Before inlining)
; CHECK: Cloning Analysis for:  foo
; CHECK: Selected generic cloning
; CHECK: Formal_0:  (Inexact)
; CHECK: i32 0
; CHECK: Formal_1:  (Inexact)
; CHECK: i32 0
; CHECK: Max clones:  4
; CHECK: Min clones:  2
; CHECK: Collecting potential constants for Formal_0
; CHECK: Added original formal:  i32 %count1
; CHECK: Binary:     %sub = sub nsw i32 %count1, 1
; CHECK: Used in IF:   %cmp = icmp eq i32 %count1, 0
; CHECK: Branch:   br i1 %cmp, label %if.then, label %if.end
; CHECK: IFSwitch: foo 1 0
; CHECK: Pending FORMAL_0
; CHECK: IFCount 1 <- 1
; CHECK: SwitchCount 0 <- 0
; CHECK: Collecting potential constants for Formal_1
; CHECK: Added original formal:  i32 %count2
; CHECK: Binary:     %sub4 = sub nsw i32 %count2, 1
; CHECK: Used in IF:   %cmp1 = icmp eq i32 %count2, 0
; CHECK: Branch:   br i1 %cmp1, label %if.then2, label %if.end3
; CHECK: IFSwitch: foo 1 0
; CHECK: Pending FORMAL_1
; CHECK: IFCount 2 <- 1
; CHECK: SwitchCount 0 <- 0
; CHECK: Possibly selecting all Pending FORMALs in Recursive Function
; CHECK: Number of argument sets found: 1
; CHECK: Skipping not profitable recursive candidate foo
; CHECK: Skipping main
; CHECK: define internal i32 @foo
; CHECK: call i32 @foo
; CHECK: define dso_local i32 @main
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; CHECK-NOT: define internal i32 @foo.1

define internal i32 @foo(i32 %count1, i32 %count2) #0 {
entry:
  %cmp = icmp eq i32 %count1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %cmp1 = icmp eq i32 %count2, 0
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  br label %return

if.end3:                                          ; preds = %if.end
  %add = add nsw i32 %count1, %count2
  %sub = sub nsw i32 %count1, 1
  %sub4 = sub nsw i32 %count2, 1
  %call = call i32 @foo(i32 %sub, i32 %sub4)
  %add5 = add nsw i32 %add, %call
  br label %return

return:                                           ; preds = %if.end3, %if.then2, %if.then
  %retval.0 = phi i32 [ 0, %if.then ], [ 0, %if.then2 ], [ %add5, %if.end3 ]
  ret i32 %retval.0
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = call i32 @foo(i32 0, i32 0)
  %call1 = call i32 @foo(i32 0, i32 0)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}


