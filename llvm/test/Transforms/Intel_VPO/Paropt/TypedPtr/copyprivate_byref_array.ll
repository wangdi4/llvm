; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; void foo(short (&a)[10]) {
; #pragma omp single copyprivate(a)
;   ;
; }

; CHECK: %[[CP_STRUCT:copyprivate.agg.*]] = alloca %__struct.kmp_copy_privates_t, align 8
; CHECK: %[[CP_A:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[CP_STRUCT]], i32 0, i32 0
; CHECK: store [10 x i16]* %a.addr.load, [10 x i16]** %[[CP_A]], align 8
; CHECK: %[[CP_STRUCT_BITCAST:.*]] = bitcast %__struct.kmp_copy_privates_t* %[[CP_STRUCT]] to i8*
; CHECK: call void @__kmpc_copyprivate(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 8, i8* %[[CP_STRUCT_BITCAST]], i8* bitcast (void (%__struct.kmp_copy_privates_t*, %__struct.kmp_copy_privates_t*)* @[[CPRIV_CALLBACK:_Z3fooRA10_s_copy_priv.*]] to i8*), i32 %{{.*}})
; CHECK: define internal void @[[CPRIV_CALLBACK]](%__struct.kmp_copy_privates_t* %[[STRUCT_DST:.*]], %__struct.kmp_copy_privates_t* %[[STRUCT_SRC:.*]])
; CHECK: %[[SRC_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_SRC]], i32 0, i32 0
; CHECK: %[[DST_GEP:.*]] = getelementptr inbounds %__struct.kmp_copy_privates_t, %__struct.kmp_copy_privates_t* %[[STRUCT_DST]], i32 0, i32 0
; CHECK: %[[A_SRC:.*]] = load [10 x i16]*, [10 x i16]** %[[SRC_GEP]]
; CHECK: %[[A_DST:.*]] = load [10 x i16]*, [10 x i16]** %[[DST_GEP]]
; CHECK: %[[A_DST_BITCAST:.*]] = bitcast [10 x i16]* %[[A_DST]] to i8*
; CHECK: %[[A_SRC_BITCAST:.*]] = bitcast [10 x i16]* %[[A_SRC]] to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 2 %[[A_DST_BITCAST]], i8* align 2 %[[A_SRC_BITCAST]], i64 20, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooRA10_s([10 x i16]* noundef nonnull align 2 dereferenceable(20) %a) {
entry:
  %a.addr = alloca [10 x i16]*, align 8
  store [10 x i16]* %a, [10 x i16]** %a.addr, align 8
  %a.addr.load = load [10 x i16]*, [10 x i16]** %a.addr, align 8
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.COPYPRIVATE"([10 x i16]* %a.addr.load) ]

  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.SINGLE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
