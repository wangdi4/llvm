; RUN: opt < %s -passes=partial-inliner -force-run-lto-partial-inline -force-enable-special-cases-partial-inline -S | FileCheck %s

; Check that the special case @foo() is partially inlined into @main

; @main looks like this after the partial inlining
; CHECK: define dso_local i32 @main() {
; CHECK: %[[R1:.*]] = load i32, i32* @myglobal, align 4
; CHECK: %[[R2:.*]] = load i1, i1* @time_check_log, align 4
; CHECK: %[[R3:.*]] = select i1 %[[R2]], i32 16383, i32 0
; CHECK: %[[R4:.*]] = and i32 %[[R3]], %[[R1]]
; CHECK: %[[R5:.*]] = icmp eq i32 %[[R4]], 0
; CHECK: br i1 %[[C:.*]], label %[[L1:.*]], label %[[L2:.*]]
; CHECK: [[L1]]:

; @foo() has the first block stubbed out in the splinter function
; CHECK: call void @foo.1.if.then
; CHECK: br label %[[L2]]
; CHECK: [[L2]]:
; CHECK: %[[R6:.*]] = phi i32
; CHECK: ret i32 %[[R6]]
; CHECK: define internal void @foo.1.if.then
; CHECK: br label %[[L3:.*]]
; CHECK: [[L3]]:
; CHECK: ret void

@time_check_log = internal unnamed_addr global i1 false, align 4

@myglobal = internal unnamed_addr global i32 0, align 4

define dso_local i32 @foo(i32 %x) #0 {
entry:
  %t2 = load i1, i1* @time_check_log, align 4
  %t3 = select i1 %t2, i32 16383, i32 0
  %t4 = and i32 %t3, %x
  %tobool = icmp eq i32 %t4, 0
  br i1 %tobool, label %if.then, label %if.end16

if.then:                                          ; preds = %entry
  br label %if.else

if.else:                                          ; preds = %if.then
  %cmp = icmp sgt i32 %x, 15
  br i1 %cmp, label %if.then1, label %if.else2

if.then1:                                         ; preds = %if.else
  %sub = sub nsw i32 15, %x
  br label %if.end16

if.else2:                                         ; preds = %if.else
  %cmp3 = icmp slt i32 %x, 15
  br i1 %cmp3, label %if.then4, label %if.else5

if.then4:                                         ; preds = %if.else2
  %add = add nsw i32 15, %x
  br label %if.end16

if.else5:                                         ; preds = %if.else2
  %cmp6 = icmp eq i32 %x, 15
  br i1 %cmp6, label %if.then7, label %if.else8

if.then7:                                         ; preds = %if.else5
  %mul = mul nsw i32 3, %x
  br label %if.end16

if.else8:                                         ; preds = %if.else5
  %cmp10 = icmp ne i32 %x, 17
  br i1 %cmp10, label %if.then11, label %if.end16

if.then11:                                        ; preds = %if.else8
  br label %if.end16

if.end16:                                         ; preds = %entry, %if.then1, %if.then4, %if.then7, %if.then11, %if.else8
  %y.0 = phi i32 [ 1, %entry ], [ %sub, %if.then1 ], [ %add, %if.then4 ], [ %mul, %if.then7 ], [ -12, %if.then11 ], [ 15, %if.else8 ]
  ret i32 %y.0
}

define dso_local i32 @main() #0 {
entry:
  %0 = load i32, i32* @myglobal, align 4
  %call = call i32 @foo(i32 %0)
  ret i32 %call
}

