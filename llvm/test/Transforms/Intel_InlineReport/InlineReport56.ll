; RUN: opt -inline -inline-report=7 < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='cgscc(inline)' -inline-report=7 < %s -S 2>&1 | FileCheck %s
; RUN: opt -inlinereportsetup -inline-report=134 < %s -S | opt -inline -inline-report=134 -S | opt -inlinereportemitter -inline-report=134 -S 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=134 < %s -S | opt -passes='cgscc(inline)' -inline-report=134 -S | opt -passes='inlinereportemitter' -inline-report=134 -S 2>&1 | FileCheck %s

target datalayout = "e-p:64:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: COMPILE FUNC: callee_frameescape
; CHECK: llvm.localescape {{.*}}Callee is intrinsic{{.*}}

; CHECK: COMPILE FUNC: caller
; CHECK: callee_frameescape {{.*}}Callee calls localescape{{.*}}

declare void @llvm.localescape(...)

define i32 @callee_frameescape(i32 %v) {
entry:
  %a = alloca i32
  call void (...) @llvm.localescape(i32* %a)
  %cmp = icmp sgt i32 %v, 2000
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %mul = mul nsw i32 %v, 10
  br label %if.then2

if.then2:
  %sub = sub i32 %v, 10
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %v2 = phi i32 [ %v, %entry ], [ %mul, %if.then2 ]
  %add = add nsw i32 %v2, 200
  ret i32 %add
}

define i32 @caller(i32 %v) {
entry:
  %r2 = call i32 @callee_frameescape(i32 %v)
  ret i32 %r2
}
