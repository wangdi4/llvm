; Test that generic cloning based on the if-switch heuristic did not occur
; because the number of ifs and/or switches was not sufficient.

; REQUIRES: asserts
; RUN: opt < %s -debug-only=ipcloning -ip-cloning -ip-cloning-after-inl -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -S 2>&1 | FileCheck %s
; RUN: opt < %s -debug-only=ipcloning -passes='module(post-inline-ip-cloning)' -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -S 2>&1 | FileCheck %s

; CHECK: Enter IP cloning: (After inlining)
; CHECK: Cloning Analysis for:  goo
; CHECK: Selected generic cloning
; CHECK: Skipping not worthy candidate goo
; CHECK: Cloning Analysis for:  hoo
; CHECK: Selected generic cloning
; CHECK: Skipping not worthy candidate hoo

; CHECK: define dso_local i32 @main
; CHECK: call i32 @goo
; CHECK: call i32 @hoo
; CHECK: define internal i32 @goo
; CHECK: define internal i32 @hoo

define dso_local i32 @main() #0 {
entry:
  %call = call i32 @goo(i32 2, i32 3, i32 4)
  %call1 = call i32 @hoo(i32 3, i32 4, i32 5)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
define internal i32 @goo(i32 %arg1, i32 %arg2, i32 %arg3) #0 {
entry:
  %retval = alloca i32, align 4
  %cmp = icmp sgt i32 %arg1, 5
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 5, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %arg2, 4
  br i1 %cmp1, label %if.then2, label %return

if.then2:                                         ; preds = %if.end
  store i32 4, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.then2, %if.then
  %t3 = load i32, i32* %retval, align 4
  ret i32 %t3
}

define internal i32 @hoo(i32 %arg1, i32 %arg2, i32 %arg3) #0 {
entry:                                           ; preds = %if.end
  %retval = alloca i32, align 4
  switch i32 %arg3, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
    i32 3, label %sw.bb
  ]

sw.bb:                                            ; preds = %entry
  store i32 3, i32* %retval, align 4
  br label %return

sw.epilog:                                        ; preds = %entry
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %sw.epilog, %sw.bb
  %t3 = load i32, i32* %retval, align 4
  ret i32 %t3
}

