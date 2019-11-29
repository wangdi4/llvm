; REQUIRES: asserts
; RUN: opt < %s -ip-cloning -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-rec-callsites=4 -debug-only=ipcloning -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-rec-callsites=4 -debug-only=ipcloning -S 2>&1 | FileCheck %s

; Test that foo is not selected for generic cloning of a recursive routine
; because its formals do not qualify under the if-switch heuristic.

; CHECK: Enter IP cloning: (Before inlining)
; CHECK: Skipping foo
; CHECK: Skipping main
; CHECK: define dso_local i32 @foo
; CHECK: call i32 @foo
; CHECK: define dso_local i32 @main
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; CHECK-NOT: define dso_local i32 @foo.1

define dso_local i32 @foo(i32 %count1, i32 %count2) #0 {
entry:
  %cmp = icmp eq i32 %count1, 0
  %cmp1 = icmp eq i32 %count2, 0
  %or.cond = and i1 %cmp, %cmp1
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %add = add nsw i32 %count1, %count2
  %sub = sub nsw i32 %count1, 1
  %sub2 = sub nsw i32 %count2, 1
  %call = call i32 @foo(i32 %sub, i32 %sub2)
  %add3 = add nsw i32 %add, %call
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 0, %if.then ], [ %add3, %if.end ]
  ret i32 %retval.0
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = call i32 @foo(i32 0, i32 0)
  %call1 = call i32 @foo(i32 0, i32 0)
  %add = add nsw i32 %call, %call1
  %call2 = call i32 @foo(i32 0, i32 0)
  %add3 = add nsw i32 %add, %call2
  %call4 = call i32 @foo(i32 0, i32 0)
  %add5 = add nsw i32 %add3, %call4
  ret i32 %add5
}


