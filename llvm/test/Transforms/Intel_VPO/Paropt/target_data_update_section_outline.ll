; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

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
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 33]
; CHECK: define dso_local void @_Z6updatePi(i32* %[[A:[^)]+]])
; CHECK: store i32* %[[A]], i32** %[[A_ADDR:[^,]+]]
; CHECK-DAG: %[[A_BASE_PTR:.+\.load]] = load i32*, i32** %[[A_ADDR]]
; CHECK-DAG: %[[A_SEC_PTR:.+]] = getelementptr i32, i32* %[[A_BASE_PTR]]
; CHECK-DAG: %[[A_BPTR_CAST:.+]] = bitcast i32* %[[A_BASE_PTR]] to i8*
; CHECK-DAG: store i8* %[[A_BPTR_CAST]]
; CHECK-DAG: %[[A_SPTR_CAST:.+]] = bitcast i32* %[[A_SEC_PTR]] to i8*
; CHECK-DAG: store i8* %[[A_SPTR_CAST]]
; Verify that the descriptor for 'a' is passed to __tgt_target_data_update
; CHECK-DAG: call void @__tgt_target_data_update(i64 -1, i32 1, {{.*}}@[[SIZE1]]{{.*}}@[[MAPTYPE1]]

; ModuleID = 'target_data_outlining.cpp'
source_filename = "target_data_outlining.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local target_declare global [100 x i32] zeroinitializer, align 16

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z6updatePi(i32* %a) #0 {
entry:
  %a.addr = alloca i32*, align 8
  store i32* %a, i32** %a.addr, align 8
  call void @_Z22modify_array_on_targetv()
  %0 = load i32*, i32** %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 13
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(), "QUAL.OMP.TO:ARRSECT"(i32** %a.addr, i64 1, i64 13, i64 17, i64 1) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  br label %target

target:
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(i32** %a.addr, i32** %a.addr, i64 8), "QUAL.OMP.MAP.TO:AGGR"(i32** %a.addr, i32* %arrayidx, i64 68) ]
  %3 = load i32*, i32** %a.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %3, i64 15
  %4 = load i32, i32* %arrayidx1, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %arrayidx1, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @_Z22modify_array_on_targetv() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline optnone uwtable "may-have-openmp-directive"="true" }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 1, !"a", i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
