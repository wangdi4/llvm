; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Original code:
; extern void modify_array_on_target();
;
; void update(int *a) {
;   modify_array_on_target();
; #pragma omp target update to(a[13:17])
; #pragma omp target map(to: a[13:17])
;   {
;     a[15]++;
;   }
; }

; CHECK-DAG: @[[SIZE1:.+]] = {{.*}} constant {{.*}} [i64 68]
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 1]
; CHECK: define dso_local void @_Z6updatePi(ptr %[[A:[^)]+]])
; CHECK-DAG: %[[A_SEC_PTR:.+]] = getelementptr inbounds i32, ptr %[[A]], i64 13
; CHECK-DAG: store ptr %[[A]]
; CHECK-DAG: store ptr %[[A_SEC_PTR]]
; Verify that the descriptor for 'a' is passed to __tgt_target_data_update
; CHECK-DAG: call void @__tgt_target_data_update(i64 %{{.*}}, i32 1, {{.*}}@[[SIZE1]]{{.*}}@[[MAPTYPE1]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @_Z6updatePi(ptr %a) #0 {
entry:
  %a.addr = alloca ptr, align 8
  %a.map.ptr.tmp = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  call void @_Z22modify_array_on_targetv()
  %0 = load ptr, ptr %a.addr, align 8
  %1 = load ptr, ptr %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 13

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.MAP.TO"(ptr %0, ptr %arrayidx, i64 68, i64 1, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.UPDATE"() ]

  %3 = load ptr, ptr %a.addr, align 8
  %4 = load ptr, ptr %a.addr, align 8
  %5 = load ptr, ptr %a.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %5, i64 13
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TO"(ptr %4, ptr %arrayidx1, i64 68, i64 33, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %a.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %4, ptr %a.map.ptr.tmp, align 8
  %7 = load ptr, ptr %a.map.ptr.tmp, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %7, i64 15
  %8 = load i32, ptr %arrayidx2, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %arrayidx2, align 4
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @_Z22modify_array_on_targetv() #1
declare token @llvm.directive.region.entry() #2
declare void @llvm.directive.region.exit(token) #2

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 12590453, !"_Z6updatePi", i32 6, i32 0, i32 0}
