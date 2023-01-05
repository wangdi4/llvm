; RUN: opt -passes=openmp-opt-cgscc -S < %s | FileCheck %s
; RUN: opt -passes='devirt<2>(cgscc(openmp-opt-cgscc))' -S < %s | FileCheck %s
;
; This test checks that kmpc library calls get appropriate attributes.
;
; CHECK: declare i32 @__kmpc_global_thread_num(%struct.ident_t* nocapture readonly)
; CHECK: declare void @__kmpc_for_static_fini(%struct.ident_t* nocapture nofree readonly, i32) [[ATTRS:#[0-9]+]]
; CHECK: declare void @__kmpc_for_static_init_4(%struct.ident_t* nocapture nofree readonly, i32, i32, i32* nocapture nofree, i32* nocapture nofree, i32* nocapture nofree, i32* nocapture nofree, i32, i32) [[ATTRS]]
; CHECK: declare void @__kmpc_for_static_init_4u(%struct.ident_t* nocapture nofree readonly, i32, i32, i32* nocapture nofree, i32* nocapture nofree, i32* nocapture nofree, i32* nocapture nofree, i32, i32) [[ATTRS]]
; CHECK: declare void @__kmpc_for_static_init_8(%struct.ident_t* nocapture nofree readonly, i32, i32, i32* nocapture nofree, i64* nocapture nofree, i64* nocapture nofree, i64* nocapture nofree, i64, i64) [[ATTRS]]
; CHECK: declare void @__kmpc_for_static_init_8u(%struct.ident_t* nocapture nofree readonly, i32, i32, i32* nocapture nofree, i64* nocapture nofree, i64* nocapture nofree, i64* nocapture nofree, i64, i64) [[ATTRS]]
; CHECK: attributes [[ATTRS]] = { nofree nounwind }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.source.0.0.31 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.32 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.31, i32 0, i32 0) }

declare i32 @__kmpc_global_thread_num(%struct.ident_t*)
declare void @__kmpc_for_static_fini(%struct.ident_t*, i32)
declare void @__kmpc_for_static_init_4(%struct.ident_t*, i32, i32, i32*, i32*, i32*, i32*, i32, i32)
declare void @__kmpc_for_static_init_4u(%struct.ident_t*, i32, i32, i32*, i32*, i32*, i32*, i32, i32)
declare void @__kmpc_for_static_init_8(%struct.ident_t*, i32, i32, i32*, i64*, i64*, i64*, i64, i64)
declare void @__kmpc_for_static_init_8u(%struct.ident_t*, i32, i32, i32*, i64*, i64*, i64*, i64, i64)

define dso_local void @foo() {
  %tid.val = call i32 @__kmpc_global_thread_num(%struct.ident_t* nonnull @.kmpc_loc.0.0.32)
  ret void
}

!llvm.module.flags = !{!0}
!0 = !{i32 7, !"openmp", i32 50}
