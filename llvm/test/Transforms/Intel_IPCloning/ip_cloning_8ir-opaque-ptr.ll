; INTEL_FEATURE_SW_ADVANCED
; Test that generic cloning based on the if-switch heuristic did not occur,
; because it is non-Intel AVX2.
; This is the same test as ip_cloning_8-opaque-ptr.ll, but checks the IR only
; without requiring asserts.

; REQUIRES: intel_feature_sw_advanced
; RUN: opt -force-opaque-pointers < %s -ip-gen-cloning-enable-morphology -ip-cloning -ip-cloning-after-inl -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt -force-opaque-pointers < %s -ip-gen-cloning-enable-morphology -passes='module(post-inline-ip-cloning)' -ip-cloning-if-heuristic -ip-cloning-switch-heuristic -ip-gen-cloning-force-if-switch-heuristic -ip-gen-cloning-min-if-count=2 -ip-gen-cloning-min-switch-count=1 -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

; CHECK: define dso_local i32 @main
; CHECK: call i32 @foo
; CHECK: call i32 @foo
; CHECK-NOT: call i32 @foo.2
; CHECK-NOT: call i32 @foo.1
; CHECK: define internal i32 @foo
; CHECK-NOT: define internal i32 @foo.1
; CHECK-NOT: define internal i32 @foo.2

define dso_local i32 @main() {
entry:
  %call = call i32 @foo(i32 2, i32 3, i32 4)
  %call1 = call i32 @foo(i32 3, i32 4, i32 5)
  %add = add nsw i32 %call, %call1
  ret i32 %add
}

define internal i32 @foo(i32 %arg1, i32 %arg2, i32 %arg3) {
entry:
  %retval = alloca i32, align 4
  %cmp = icmp sgt i32 %arg1, 5
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 5, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  %cmp1 = icmp slt i32 %arg2, 4
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  store i32 4, ptr %retval, align 4
  br label %return

if.end3:                                          ; preds = %if.end
  switch i32 %arg3, label %sw.epilog [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
    i32 3, label %sw.bb
  ]

sw.bb:                                            ; preds = %if.end3, %if.end3, %if.end3
  store i32 3, ptr %retval, align 4
  br label %return

sw.epilog:                                        ; preds = %if.end3
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %sw.epilog, %sw.bb, %if.then2, %if.then
  %t3 = load i32, ptr %retval, align 4
  ret i32 %t3
}
; end INTEL_FEATURE_SW_ADVANCED
