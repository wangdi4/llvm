; This test verifies that "store i32 0, ptr %i" is not removed by
; InstCombine by using incorrect points-to info.
; Points-to info for %i should be Universal as getLZsLNs.gLZsLNs
; variable is escaped in "@getLZsLNs" function.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK-LABEL: @CreateRealZones
; CHECK: store i32 0, ptr %i

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@getLZsLNs.gLZsLNs = internal global ptr null

define ptr @getLZsLNs() {
entry:
  ret ptr @getLZsLNs.gLZsLNs
}

define void @CreateRealZones() {
entry:
  %i = load ptr, ptr @getLZsLNs.gLZsLNs, align 8
  store i32 0, ptr %i, align 4
  ret void
}
