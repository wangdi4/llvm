; RUN: opt -S -csa-cache-localizer -csa-memop-ordering <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_local_cache(i32* %arg0, i32* %arg1, i32 %len) {
; CHECK-LABEL: test_local_cache
entry:
; CHECK: entry:
; CHECK: %[[MEMENTRY:[a-z0-9_.]+]] = call i1 @llvm.csa.mementry()

  store i32 0, i32* %arg0
; CHECK: call void @llvm.csa.inord(i1 %[[MEMENTRY]])
; CHECK-NEXT: store i32 0, i32* %arg0
; CHECK-NEXT: %[[STORE1:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  %cache = call i32 @llvm.csa.local.cache.region.begin(i32 1024, i32 5)
; CHECK: call void @llvm.csa.inord(i1 %[[STORE1]])
; CHECK-NEXT: %cache = call i32 @llvm.csa.local.cache.region.begin(i32 1024, i32 1), !CSA.Local.Cache.ID !0
; CHECK-NEXT: %[[BEGIN:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()
  br label %loop

loop:
; CHECK: loop:
  %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
; CHECK: %i = phi i32 [ 0, %entry ], [ %ip1, %loop ]
; CHECK-NEXT: %[[PHI:[a-z0-9_.]+]] = phi i1 [ %[[BEGIN]], %entry ], [ %[[STORE2:[a-z0-9_.]+]], %loop ]

  %loaded = load i32, i32* %arg0
; CHECK: call void @llvm.csa.inord(i1 %[[PHI]])
; CHECK-NEXT: %loaded = load i32, i32* %arg0, !CSA.Local.Cache.ID !0
; CHECK-NEXT: %[[LOADED:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  store i32 %loaded, i32* %arg1
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: store i32 %loaded, i32* %arg1, !CSA.Local.Cache.ID !0
; CHECK-NEXT: %[[STORE2]] = call i1 @llvm.csa.outord()

  %ip1 = add nuw i32 %i, 1
  %reloop = icmp ne i32 %ip1, %len
  br i1 %reloop, label %loop, label %exit

exit:
; CHECK: exit:
  call void @llvm.csa.local.cache.region.end(i32 %cache, i32 5)
; CHECK: call void @llvm.csa.inord(i1 %[[STORE2]])
; CHECK-NEXT: call void @llvm.csa.local.cache.region.end(i32 %cache, i32 1), !CSA.Local.Cache.ID !0
; CHECK-NEXT: %[[END:[a-z0-9_.]+]] = call i1 @llvm.csa.outord()

  ret void
}

declare i32 @llvm.csa.local.cache.region.begin(i32, i32)
declare void @llvm.csa.local.cache.region.end(i32, i32)