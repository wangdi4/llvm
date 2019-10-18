; REQUIRES: asserts
; RUN: opt < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -ip-cloning -debug-only=ipcloning 2>&1 | FileCheck %s
; RUN: opt < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -passes='module(ip-cloning)' -debug-only=ipcloning 2>&1 | FileCheck %s

; Check that foo is selected for cloning as a "many recursive calls" cloning
; candidate.

; Check the -ip-cloning trace output

; CHECK: MRC Cloning: Testing: foo
; CHECK: MRC Cloning: IF ARG #0
; CHECK: MRC Cloning: IF ARG #1
; CHECK: MRC Cloning: SWITCH ARG #2
; CHECK: MRC Cloning: GOOD IF CB: goo   %call = call i32 @foo(i32 1, i32 1, i32 %1)
; CHECK: MRC Cloning: GOOD SWITCH CB: goo   %call = call i32 @foo(i32 1, i32 1, i32 %1)
; CHECK: MRC Cloning: BEST CB: goo   %call = call i32 @foo(i32 1, i32 1, i32 %1)
; CHECK: MRC Cloning: OK: foo
; CHECK: Selected many recursive calls cloning
; CHECK: MRC Cloning: foo TO foo.1

; Check changes to the IR

@myglobal = dso_local global i32 45, align 4

%struct.MYSTRUCT = type { i32, i32 }
@cache = dso_local global %struct.MYSTRUCT zeroinitializer, align 4

define dso_local i32 @goo(%struct.MYSTRUCT* %cacheptr) {
entry:
  %0 = load i32, i32* @myglobal, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %field2 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %cacheptr, i32 0, i32 1
  %1 = load i32, i32* %field2, align 4
  %call = call i32 @foo(i32 1, i32 1, i32 %1)
  br label %return

if.end:                                           ; preds = %entry
  %2 = load i32, i32* @myglobal, align 4
  %call1 = call i32 @foo(i32 0, i32 %2, i32 0)
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %call, %if.then ], [ %call1, %if.end ]
  ret i32 %retval.0
}

; Check that a test is inserted for the switch variable, which results in
; either a call to foo or the clone foo.1.

; CHECK: define dso_local i32 @goo{{.*}}
; CHECK: [[V1:%[A-Za-z0-9.]+]] = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %cacheptr, i32 0, i32 1
; CHECK: [[V2:%[A-Za-z0-9.]+]] = load i32, i32* [[V1]], align 4
; CHECK: [[V3:%[A-Za-z0-9.]+]] = icmp eq i32 [[V2]], 0
; CHECK: br i1 [[V3]], label %[[V4:[A-Za-z0-9.]+]], label %[[V5:[A-Za-z0-9.]+]]
; CHECK: [[V5]]:
; CHECK: {{.*}}call i32 @foo(i32 1, i32 1, i32 %1)
; CHECK: br label %[[V6:[A-Za-z0-9.]+]]
; CHECK: [[V4]]:
; CHECK: {{.*}}call i32 @foo.1(i32 1, i32 1, i32 0)
; CHECK: br label %[[V6]]
; CHECK: [[V6]]:

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

; Check that a test is inserted at the beginning of foo, calling the clone
; under the appropriate circumstances.

; CHECK: define internal i32 @foo{{.*}}
; CHECK: entry:
; CHECK: [[V1:%[A-Za-z0-9.]+]] = icmp eq i32 %arg0, 1
; CHECK: [[V2:%[A-Za-z0-9.]+]] = icmp eq i32 %arg1, 1
; CHECK: [[V3:%[A-Za-z0-9.]+]] = and i1 [[V1]], [[V2]]
; CHECK: [[V4:%[A-Za-z0-9.]+]] = icmp eq i32 %arg2, 0
; CHECK: [[V5:%[A-Za-z0-9.]+]] = and i1 [[V3]], [[V4]]
; CHECK: br i1 [[V5]], label %[[V6:[0-9]+]], label %[[V7:[0-9]+]]
; CHECK: [[V6]]:
; CHECK: [[V7:%[A-Za-z0-9.]+]] = call i32 @foo.1{{.*}}
; CHECK:  ret i32 [[V7]]

; Check that the clone foo.1 is generated

; CHECK: define internal i32 @foo.1{{.*}}
; CHECK: entry:

; Check that the IF-tests get their constant values of 1.
; CHECK: {{.*}} = icmp ne i32 1, 0
; CHECK: {{.*}} = icmp ne i32 1, 0

; Check that the SWITCH-test gets its constant value of 0.
; CHECK: switch i32 0, label %sw.default
; CHECK: sw.default:

; Check that the recursive call in the switch is transformed into a call to
; the clone.
; CHECK: {{.*}} call i32 @foo.1(i32 1, i32 1, i32 0)
; CHECK: sw.bb:

; Check that the recursive call outside the default switch case is not
; transformed.
; CHECK: [[V1:%[A-Za-z0-9.]+]] = sub nsw i32 0, 2
; CHECK: {{.*}} = call i32 @foo(i32 0, i32 1, i32 [[V1]])
; CHECK: br label %[[V2:[A-Za-z0-9]+]]
; CHECK: [[V2]]:

; Check that the recursive call outside the SWITCH-test is transformed into
; a conditional call of either the original or the clone.
; CHECK: [[V3:%[A-Za-z0-9.]+]] = icmp eq i32 0, 1
; CHECK: br i1 [[V3]], label %[[V4:[A-Za-z0-9.]+]], label %[[V5:[A-Za-z0-9.]+]]
; CHECK: [[V5]]:
; CHECK: {{.*}} = call i32 @foo(i32 0, i32 1, i32 0)
; CHECK: br label %[[V6:[0-9]+]]
; CHECK: [[V4]]:
; CHECK: {{.*}} call i32 @foo.1(i32 1, i32 1, i32 0)
; CHECK: br label %[[V6]]
; CHECK: [[V6]]:

define dso_local i32 @main() #0 {
entry:
  %call = call i32 @goo(%struct.MYSTRUCT* @cache)
  ret i32 %call
}

