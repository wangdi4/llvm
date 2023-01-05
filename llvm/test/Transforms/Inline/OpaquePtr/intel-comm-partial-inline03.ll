; RUN: opt -opaque-pointers < %s -passes=partial-inliner -force-run-lto-partial-inline -force-enable-special-cases-partial-inline -S | FileCheck %s

; Check that the special case @foo() is not partially inlined into @main
; because there are not enough join points at the @foo's return block.

; CHECK: define dso_local i32 @foo
; CHECK: define dso_local i32 @main()
; Check that no other function is emitted
; CHECK-NOT: define

@time_check_log = internal unnamed_addr global i1 false, align 4
@myglobal = internal unnamed_addr global i32 0, align 4

define dso_local i32 @foo(i32 %x) {
entry:
  %t2 = load i1, ptr @time_check_log, align 4
  %t3 = select i1 %t2, i32 16383, i32 0
  %t4 = and i32 %t3, %x
  %tobool = icmp eq i32 %t4, 0
  br i1 %tobool, label %if.then, label %if.end16

if.then:                                          ; preds = %entry
  br label %if.else

if.else:                                          ; preds = %if.then
  %cmp = icmp sgt i32 %x, 15
  br label %if.end16

if.end16:                                         ; preds = %if.else, %entry
  %y.0 = phi i32 [ 1, %entry ], [ 15, %if.else ]
  ret i32 %y.0
}

define dso_local i32 @main() {
entry:
  %i = load i32, ptr @myglobal, align 4
  %call = call i32 @foo(i32 %i)
  ret i32 %call
}
