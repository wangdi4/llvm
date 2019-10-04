; RUN: opt < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -ip-cloning 2>&1 | FileCheck %s
; RUN: opt < %s -S -ip-manyreccalls-cloning-min-rec-callsites=2 -passes='module(ip-cloning)' 2>&1 | FileCheck %s

; Check that foo is not selected for cloning as a "many recursive calls"
; cloning candidate, because it is varargs.
; This is the same test as ip_cloning_mrc03.ll, but checks for IR without
; requiring asserts.

; Check that no recursive clone of foo is made.
; CHECK-NOT: define internal i32 @foo.1
; Check that no call is made to a recursive clone of foo.
; CHECK-NOT: {{.*}}call @foo.1

declare i32 @llvm.va_arg_pack()

@myglobal = dso_local global i32 45, align 4

%struct.MYSTRUCT = type { i32, i32 }
@cache = dso_local global %struct.MYSTRUCT zeroinitializer, align 4

define internal i32 @foo(i32 %arg0, i32 %arg1, i32 %arg2, ...) {
entry:
  %arg3 = tail call i32 @llvm.va_arg_pack()
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
  %call = tail call i32 (i32, i32, i32, ...) @foo(i32 1, i32 1, i32 %arg2, i32 %arg3)
  br label %return

sw.bb:                                            ; preds = %if.end, %if.end
  %sub2 = sub nsw i32 %arg2, 2
  %call3 = tail call i32 (i32, i32, i32, ...) @foo(i32 0, i32 1, i32 %sub2, i32 %arg3)
  br label %return

return:                                           ; preds = %sw.bb, %sw.default, %if.then
  %call4 = call i32 (i32, i32, i32, ...) @foo(i32 %arg2, i32 1, i32 0, i32 %arg3)
  ret i32 %call4
}

define dso_local i32 @goo(%struct.MYSTRUCT* %cacheptr) {
entry:
  %0 = load i32, i32* @myglobal, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %field2 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %cacheptr, i32 0, i32 1
  %1 = load i32, i32* %field2, align 4
  %call = tail call i32 (i32, i32, i32, ...) @foo(i32 1, i32 1, i32 %1, i32 7)
  br label %return

if.end:                                           ; preds = %entry
  %2 = load i32, i32* @myglobal, align 4
  %call1 = tail call i32 (i32, i32, i32, ...) @foo(i32 0, i32 %2, i32 0, i32 8)
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ %call, %if.then ], [ %call1, %if.end ]
  ret i32 %retval.0
}

define dso_local i32 @main() #0 {
entry:
  %call = call i32 @goo(%struct.MYSTRUCT* @cache)
  ret i32 %call
}

