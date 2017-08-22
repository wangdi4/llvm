; RUN: opt %s -latesimplifycfg -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Switch case has the same constant integer result as the default case.
define i32 @test(i32 %i) {
; CHECK-LABEL: @test
; CHECK-LABEL: entry:
; CHECK-NEXT: %cond = icmp eq i32 %i, 2
; CHECK-NEXT: select i1 %cond, i32 1, i32 0
; CHECK-NEXT: ret i32
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb.1
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb.1:                                          ; preds = %entry
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb.1, %sw.bb
  %a.0 = phi i32 [ 0, %sw.default ], [ 1, %sw.bb.1 ], [ 0, %sw.bb ]
  ret i32 %a.0
}

; Switch case has the same successor as the default case
define i32 @test1(i32 %i) {
; CHECK-LABEL: @test1
; CHECK-LABEL: entry:
; CHECK-NEXT: %cond = icmp eq i32 %i, 1
; CHECK-NEXT: select i1 %cond, i32 3, i32 0
; CHECK-NEXT: ret i32
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.default
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb.2, %sw.bb.1, %sw.bb
  %a.0 = phi i32 [ 0, %sw.default ], [ 3, %sw.bb ]
  ret i32 %a.0
}

; Switch case has the same constant FP result as the default case.
define double @test2(i32 %i) {
; CHECK-LABEL: @test2
; CHECK-LABEL: entry:
; CHECK-NEXT: %cond = icmp eq i32 %i, 2
; CHECK-NEXT: select i1 %cond, double
; CHECK-NEXT: ret double
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb.1
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb.1:                                          ; preds = %entry
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb.1, %sw.bb
  %a.0 = phi double [ 3.141590e+00, %sw.default ], [ 1.000000e-02, %sw.bb.1 ], [ 3.141590e+00, %sw.bb ]
  ret double %a.0
}

; Switch case has the same result value as the default case.
declare i32 @abc(...)

define i32 @test3(i32 %i) {
; CHECK-LABEL: @test3
; CHECK-LABEL: entry:
; CHECK-NEXT: %call = call i32 (...) @abc()
; CHECK-NEXT: %cond = icmp eq i32 %i, 2
; CHECK-NEXT: select i1 %cond, i32 0, i32 %call
; CHECK-NEXT: ret i32
entry:
  %call = call i32 (...) @abc()
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb.1
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb.1:                                          ; preds = %entry
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb.1, %sw.bb
  %b.0 = phi i32 [ %call, %sw.default ], [ 0, %sw.bb.1 ], [ %call, %sw.bb ]
  ret i32 %b.0
}

; Switch statement should still be converted to jump table when appropriate
; after some cases are removed.
define i32 @test4(i32 %i) {
; CHECK-LABEL: @test4
; CHECK-LABEL: entry:
; CHECK-NEXT: %switch.tableidx = sub i32 %i, 1
; CHECK-NEXT: icmp ult i32 %switch.tableidx, 4
entry:
  switch i32 %i, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb.1
    i32 3, label %sw.bb.2
    i32 4, label %sw.bb.3
    i32 5, label %sw.bb.4
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb.1:                                          ; preds = %entry
  br label %sw.epilog

sw.bb.2:                                          ; preds = %entry
  br label %sw.epilog

sw.bb.3:                                          ; preds = %entry
  br label %sw.epilog

sw.bb.4:                                          ; preds = %entry
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb.4, %sw.bb.3, %sw.bb.2, %sw.bb.1, %sw.bb
  %a.0 = phi i32 [ 8, %sw.default ], [ 8, %sw.bb.4 ], [ 15, %sw.bb.3 ], [ 100, %sw.bb.2 ], [ 7, %sw.bb.1 ], [ 5, %sw.bb ]
  ret i32 %a.0
}

