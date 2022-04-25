; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-output 2>&1 | FileCheck %s

; Verify that we create 2 regions with entry blocks bb404 and bb455 belonging to
; 2 different loops. Previously, we were tracing back to bb339 to associate the
; same region entry directive with both loops.


; CHECK: Region 1
; CHECK: EntryBB: %bb404

; CHECK: Region 2
; CHECK: EntryBB: %bb455


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %arg4) {
bb:
  br label %bb339

bb339:                                            ; preds = %bb
  %i340 = call token @llvm.directive.region.entry() #1 [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(i8* null), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  switch i32 %arg4, label %bb359 [
    i32 4, label %bb401
    i32 2, label %bb358
  ]

bb358:                                            ; preds = %bb339
  unreachable

bb359:                                            ; preds = %bb339
  br label %bb455

bb401:                                            ; preds = %bb339
  br label %bb404

bb404:                                            ; preds = %bb404, %bb401
  %i454 = icmp ult i64 undef, undef
  br i1 %i454, label %bb404, label %bb502

bb455:                                            ; preds = %bb455, %bb359
  %i500 = icmp ult i64 undef, undef
  br i1 %i500, label %bb455, label %bb501

bb501:                                            ; preds = %bb455
  br label %bb504

bb502:                                            ; preds = %bb404
  br label %bb504

bb504:                                            ; preds = %bb502, %bb501
  call void @llvm.directive.region.exit(token %i340) #1 [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


