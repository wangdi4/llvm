; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-tpv -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt-tpv' -S %s | FileCheck %s

; Make sure that vpo-paropt-tpv creates the local version of `@global` in the beginning
; of the routine, and uses it in both uses in the function, including one in a PHI.

; CHECK: @__tpv_ptr_global = internal global ptr null, align 64
; CHECK: define void @widget() {
; CHECK: bb:
; CHECK:   %global.tpv.cached.addr = alloca ptr, align 8
; CHECK:   %tid.val = tail call i32 @__kmpc_global_thread_num(ptr @.kmpc_loc{{.*}})
; CHECK:   [[TPV_CACHED:%[^ ]+]] = call ptr @__kmpc_threadprivate_cached(ptr @.kmpc_loc{{.*}}, i32 %tid.val, ptr @global, i64 4, ptr @__tpv_ptr_global)
; CHECK:   store ptr [[TPV_CACHED]], ptr %global.tpv.cached.addr, align 8
; CHECK:   %global.tpv.cached = load ptr, ptr %global.tpv.cached.addr, align 8
;
; CHECK: bb2:                                              ; preds = %bb1, %tid.bb
; CHECK:   %tmp = phi ptr [ null, %bb1 ], [ %global.tpv.cached, %tid.bb ]
; CHECK:   %tmp1 = ptrtoint ptr %global.tpv.cached to i64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = external thread_private global i32

define void @widget() {
bb:
  br label %bb2

bb1:                                              ; No predecessors!
  br label %bb2

bb2:                                              ; preds = %bb1, %bb
  %tmp = phi ptr [ null, %bb1 ], [ @global, %bb]
  %tmp1 = ptrtoint ptr @global to i64
  ret void
}
