; RUN: opt -passes='default<O3>' -paropt=31 -disable-output -pass-remarks-analysis=openmp %s 2>&1 | FileCheck %s
;
; Test src:
;
; struct S {
;   int h, i, j;
;   int ag, ah;
; };
;
; extern void foo(S bbox);
;
; void bar(const S &bbox) {
;   foo(bbox);
; }
;
; void test() {
;   S bbox;
; #pragma omp target
;   bar(bbox);
; }

; Check that shared privatization pass can determine that 'bbox' can be passed
; to the target region as firstprivate.
;
; CHECK: remark: {{.+}} MAP:TOFROM clause for variable 'bbox' on 'target' construct can be changed to FIRSTPRIVATE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S = type { i32, i32, i32, i32, i32 }

define dso_local void @_Z3barRK1S(ptr nonnull align 4 dereferenceable(20) %bbox) {
entry:
  %bbox.addr = alloca ptr, align 8
  %agg.tmp = alloca %struct.S, align 8
  store ptr %bbox, ptr %bbox.addr, align 8
  %0 = load ptr, ptr %bbox.addr, align 8
  call void @llvm.memcpy.p0i8.p0i8.i64(ptr align 4 %agg.tmp, ptr align 4 %0, i64 20, i1 false)
  call void @_Z3foo1S(ptr byval(%struct.S) align 8 %agg.tmp)
  ret void
}

declare dso_local void @_Z3foo1S(ptr byval(%struct.S) align 8)

declare void @llvm.memcpy.p0i8.p0i8.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

define dso_local void @_Z4testv() "may-have-openmp-directive"="true" {
entry:
  %bbox = alloca %struct.S, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %bbox, ptr %bbox, i64 20, i64 547, ptr null, ptr null) ] ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  call void @_Z3barRK1S(ptr nonnull align 4 dereferenceable(20) %bbox)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -676547204, !"_Z4testv", i32 14, i32 0, i32 0}
