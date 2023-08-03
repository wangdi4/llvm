; RUN: opt -passes="loop(loop-idiom),function(instcombine)" -S < %s | FileCheck %s

; CHECK-NOT: lcssa{{.*}}phi

; SCEVExpander (called from loop-idiom) is "leaking" LCSSA phis (creating
; cross-loop LCSSA phis which cannot automatically be deleted), which was
; causing an assert.
; This behavior should be fairly harmless as long as IC can still delete them.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

%struct.hoge = type { %struct.widget, i32 }
%struct.widget = type { i64 }

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

define dso_local void @quux() local_unnamed_addr #1 align 2 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb5, %bb
  %tmp = phi i64 [ %tmp2, %bb5 ], [ 0, %bb ]
  %tmp2 = add nuw nsw i64 %tmp, 1
  br i1 undef, label %bb6, label %bb3

bb3:                                              ; preds = %bb1
  switch i64 undef, label %bb5 [
    i64 -4096, label %bb4
    i64 -8192, label %bb4
  ]

bb4:                                              ; preds = %bb3, %bb3
  br label %bb6

bb5:                                              ; preds = %bb3
  br label %bb1

bb6:                                              ; preds = %bb4, %bb1
  %tmp7 = phi i64 [ %tmp2, %bb4 ], [ %tmp2, %bb1 ]
  %tmp9 = getelementptr inbounds %struct.hoge, ptr undef, i64 undef
  %tmp10 = getelementptr inbounds %struct.hoge, ptr %tmp9, i64 %tmp7
  br label %bb11

bb11:                                             ; preds = %bb11, %bb6
  %tmp12 = phi ptr [ %tmp17, %bb11 ], [ undef, %bb6 ]
  %tmp13 = phi ptr [ %tmp16, %bb11 ], [ %tmp10, %bb6 ]
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %tmp13, ptr noundef nonnull align 8 dereferenceable(16) %tmp12, i64 16, i1 false) #2
  %tmp16 = getelementptr inbounds %struct.hoge, ptr %tmp13, i64 1
  %tmp17 = getelementptr inbounds %struct.hoge, ptr %tmp12, i64 1
  %tmp18 = icmp eq ptr %tmp17, undef
  br i1 %tmp18, label %bb19, label %bb11

bb19:                                             ; preds = %bb11
  ret void
}

attributes #0 = { argmemonly nofree nounwind willreturn }
attributes #1 = { "tune-cpu"="generic" }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
