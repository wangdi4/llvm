; RUN: llc -mtriple=x86_64-unknown-unknown -disable-cgp-delete-phis -cgp-split-switch-critical-edge=true -start-after=indirectbr-expand -stop-after=codegenprepare -o - %s | FileCheck %s
;
; 27123: Interaction between div opt and critical edge removal was crashing the
; compiler.

; CHECK: .split:{{.*}}preds = %sw.bb3.i, %sw_exit
; CHECK-NEXT: = or i64
; CHECK-NEXT: = and i64
; CHECK-NEXT: = icmp eq i64
; CHECK-NEXT: br i1{{.*}}label{{.*}}label

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden void @test(i64 %Str.coerce1, i32 %Kind) local_unnamed_addr #0 align 2 {
entry:
  switch i32 %Kind, label %sw_exit [
    i32 4, label %sw.bb5.i
    i32 3, label %sw.bb3.i
    i32 1, label %sw.bb1.i
  ]

sw.bb1.i:                                         ; preds = %entry
  unreachable

sw.bb3.i:                                         ; preds = %entry
  br label %sw_exit

sw.bb5.i:                                         ; preds = %entry
  unreachable

sw_exit: ; preds = %sw.bb3.i, %entry
  %CharByteWidth.0.i = phi i32 [ 0, %entry ], [ undef, %sw.bb3.i ]
  %div = udiv i64 %Str.coerce1, undef
  switch i32 undef, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb30
    i32 4, label %sw.bb40
  ]

sw.bb:                                            ; preds = %sw_exit
  unreachable

sw.bb30:                                          ; preds = %sw_exit
  ret void

sw.bb40:                                          ; preds = %sw_exit
  %conv43 = shl i64 %div, 2
  unreachable

sw.default:                                       ; preds = %sw_exit
  unreachable
}

attributes #0 = { "tune-cpu"="generic" }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
