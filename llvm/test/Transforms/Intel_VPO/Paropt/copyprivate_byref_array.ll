; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; void foo(short (&a)[10]) {
; #pragma omp single copyprivate(a)
;   ;
; }

; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[CP_STRUCT]], i32 0, i32 0
; CHECK: store ptr %a.addr.load, ptr %[[CP_A]], align 8
; CHECK: call void @__kmpc_copyprivate(ptr @{{.*}}, i32 %{{.*}}, i32 8, ptr %[[CP_STRUCT]], ptr @[[CPRIV_CALLBACK:_Z3fooRA10_s_copy_priv.*]], i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](ptr %[[STRUCT_DST:.*]], ptr %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, ptr %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[A_SRC:.*]] = load ptr, ptr %[[SRC_GEP]]
; CHECK: %[[A_DST:.*]] = load ptr, ptr %[[DST_GEP]]
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 2 %[[A_DST]], ptr align 2 %[[A_SRC]], i64 20, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooRA10_s(ptr noundef nonnull align 2 dereferenceable(20) %a) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  %a.addr.load = load ptr, ptr %a.addr, align 8
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE:TYPED"(ptr %a.addr.load, [10 x i16] zeroinitializer, i32 1) ]

  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.SINGLE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
