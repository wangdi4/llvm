; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -passes='module(ip-cloning)' -ip-manyreccalls-splitting=false 2>&1 | FileCheck %s

; Check that foo is not selected for cloning as a "many recursive calls"
; cloning candidate, because no best callsite is identified.
; This is the same test as ip_cloning_mrc06-opaque-ptr.ll, but checks for IR
; without requiring asserts.

; CHECK: define internal i32 @foo
; Check that no recursive clone of foo is made.
; CHECK-NOT: define internal i32 @foo.1
; Check that no call is made to a recursive clone of foo.
; CHECK-NOT: {{.*}}call @foo.1

%struct.MYSTRUCT = type { i32, i32 }

@myglobal = dso_local global i32 45, align 4
@cache = dso_local global %struct.MYSTRUCT zeroinitializer, align 4

define dso_local i32 @goo(ptr %cacheptr) {
entry:
  %i = load i32, ptr @myglobal, align 4
  %tobool = icmp ne i32 %i, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %field2 = getelementptr inbounds %struct.MYSTRUCT, ptr %cacheptr, i32 0, i32 1
  %i1 = load i32, ptr %field2, align 4
  %call = call i32 @foo(i32 0, i32 %i1, i32 %i1)
  br label %return

if.end:                                           ; preds = %entry
  %i2 = load i32, ptr @myglobal, align 4
  %call1 = call i32 @foo(i32 1, i32 0, i32 0)
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %call, %if.then ], [ %call1, %if.end ]
  ret i32 %retval.0
}

define internal i32 @foo(i32 %arg0, i32 %arg1, i32 %arg2) {
entry:
  %tobool = icmp ne i32 %arg0, 0
  br i1 %tobool, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %tobool1 = icmp ne i32 %arg1, 0
  br i1 %tobool1, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  br label %return

if.end:                                           ; preds = %land.lhs.true, %entry
  switch i32 %arg2, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
  ]

sw.default:                                       ; preds = %if.end
  %call = call i32 @foo(i32 1, i32 1, i32 %arg2)
  br label %return

sw.bb:                                            ; preds = %if.end, %if.end
  %sub2 = sub nsw i32 %arg2, 2
  %call3 = call i32 @foo(i32 0, i32 1, i32 %sub2)
  br label %return

return:                                           ; preds = %sw.bb, %sw.default, %if.then
  %call4 = call i32 @foo(i32 %arg2, i32 1, i32 0)
  ret i32 %call4
}

define dso_local i32 @main() {
entry:
  %call = call i32 @goo(ptr @cache)
  ret i32 %call
}
; end INTEL_FEATURE_SW_ADVANCED
