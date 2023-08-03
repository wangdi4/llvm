; REQUIRES: asserts
; This test checks if whole program is seen when indirect calls are present.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-assume-executable -whole-program-assume-read -whole-program-assume-hidden -o %t2 %t1 2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  DETECTED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @action(ptr %a, i32 %b) {
entry:
  %res = tail call i32 %a(i32 %b)
  ret i32 %res
}

define i32 @load_action(ptr %a, i32 %b) {
entry:
  %fun = load ptr, ptr %a
  %res = tail call i32 %fun(i32 %b)
  ret i32 %res
}

@al_sub = internal dso_local alias i32 (i32), ptr @sub

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
ntry:
  %cmp = icmp sgt i32 %argc, 2
  %rem1.i = and i32 %argc, 1
  %tobool.i = icmp eq i32 %rem1.i, 0
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:
  %cond.i = select i1 %tobool.i, ptr @sub, ptr @add
  br label %cond.end

cond.false:
  %cond.i8 = select i1 %tobool.i, ptr @add, ptr @al_sub
  br label %cond.end

cond.end:
  %cond = phi ptr [ %cond.i, %cond.true ], [ %cond.i8, %cond.false ]
  %call2 = call i32 %cond(i32 %argc)
  %call3 = call i32 @action(ptr %cond, i32 %call2)
  %addr = inttoptr i32 %argc to ptr
  %call4 = call i32 @load_action(ptr %addr, i32 %call3)
  %fun = inttoptr i32 %argc to ptr
  %call5 = call i32 %fun(i32 %call4)
  %bc = bitcast ptr @add to ptr
  %call6 = call i32 %bc(i32 %argc, i32 %argc)
  %fun2 = inttoptr i32 1 to ptr
  %call7 = call i32 %fun2(i32 %call6)
  %fun3 = select i1 %tobool.i, ptr %fun2, ptr undef
  %call8 = call i32 %fun3(i32 %call7)
  ret i32 %call8
}
