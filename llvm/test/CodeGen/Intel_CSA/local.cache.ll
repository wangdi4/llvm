; RUN: llc -mtriple=csa <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_local_cache(i32* %arg0, i32* %arg1, i32 %len) {
entry:
  store i32 0, i32* %arg0
  %cache = call i32 @llvm.csa.local.cache.region.begin(i32 1024, i32 5)
  br label %loop
; CHECK: st32    %[[DUMMY0:[a-z0-9_.]+]], 0, %[[OUTORD0:[a-z0-9_.]+]], %[[DUMMY1:[a-z0-9_.]+]], MEMLEVEL_T0
; CHECK: .cache [[CACHE:[a-z0-9_.]+]][1024]
; CHECK-NEXT: lfence  %[[OUTORD1:[a-z0-9_.]+]], %[[OUTORD0]], [[CACHE]]

loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
  %loaded = load i32, i32* %arg0
  store i32 %loaded, i32* %arg1
  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit
; CHECK: pick0   %[[PICK0:[a-z0-9_.]+]], %[[DUMMY4:[a-z0-9_.]+]], %[[DUMMY5:[a-z0-9_.]+]], %[[OUTORD1]]
; CHECK: ld32    %[[DUMMY2:[a-z0-9_.]+]], %[[DUMMY3:[a-z0-9_.]+]], %ign, %[[PICK0]], MEMLEVEL_T0, %ign, [[CACHE]]
; CHECK: st32    %[[DUMMY5:[a-z0-9_.]+]], %[[DUMMY6:[a-z0-9_.]+]], %[[OUTORD2:[a-z0-9_.]+]], %ign, MEMLEVEL_T0, %ign, [[CACHE]]
; CHECK: switch0 %[[DUMMY8:[a-z0-9_.]+]], %[[OUTORD3:[a-z0-9_.]+]], %[[DUMMY9:[a-z0-9_.]+]], %[[OUTORD2]]
; CHECK: lfence  %[[DUMMY7:[a-z0-9_.]+]], %[[OUTORD3]], [[CACHE]]


exit:
  call void @llvm.csa.local.cache.region.end(i32 %cache, i32 5)
  ret void
}

declare i32 @llvm.csa.local.cache.region.begin(i32, i32)
declare void @llvm.csa.local.cache.region.end(i32, i32)