; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Original code:
; #pragma omp declare target
; int a[100];
; #pragma omp end declare target
;
; extern void modify_array_on_target();
;
; void enter_exit_data() {
; #pragma omp target enter data map(always,to: a[7:17))
;   modify_array_on_target();
; #pragma omp target exit data map(always,from: a[9:13])
; }

; CHECK-DAG: @[[SIZE1:.+]] = {{.*}} constant {{.*}} [i64 52]
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 38]
; CHECK-DAG: @[[SIZE2:.+]] = {{.*}} constant {{.*}} [i64 68]
; CHECK-DAG: @[[MAPTYPE2:.+]] = {{.*}} constant {{.*}} [i64 37]
; CHECK-DAG: %[[GEP1:.+]] = {{.*}}getelementptr {{.*}} @a, i64 0, i64 7
; CHECK-DAG: %[[A1:.+]] = bitcast i8*{{.*}}@a
; CHECK-DAG: %[[CAST1_1:.+]] = bitcast {{.*}} %[[A1]] to i8*
; CHECK-DAG: store i8* %[[CAST1_1]]
; CHECK-DAG: %[[CAST1_2:.+]] = bitcast i32* %[[GEP1]] to i8*
; CHECK-DAG: store i8* %[[CAST1_2]]
; Verify that the descriptor for 'a' is passed to __tgt_target_data_begin
; CHECK-DAG: call void @__tgt_target_data_begin(i64 -1, i32 1, {{.*}}@[[SIZE2]]{{.*}}@[[MAPTYPE2]]

; Verify that the descriptor for 'a' is passed to __tgt_target_data_exit
; CHECK-DAG: %[[GEP2:.+]] = {{.*}}getelementptr {{.*}} @a, i64 0, i64 9
; CHECK-DAG: %[[A2:.+]] = bitcast i8*{{.*}}@a
; CHECK-DAG: %[[CAST2_1:.+]] = bitcast {{.*}} %[[A2]] to i8*
; CHECK-DAG: store i8* %[[CAST2_1]]
; CHECK-DAG: %[[CAST2_2:.+]] = bitcast i32* %[[GEP2]] to i8*
; CHECK-DAG: store i8* %[[CAST2_2]]
; CHECK-DAG: call void @__tgt_target_data_end(i64 -1, i32 1, {{.*}}@[[SIZE1]]{{.*}}@[[MAPTYPE1]]

; ModuleID = 'target_data_outlining.cpp'
source_filename = "target_data_outlining.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local target_declare global [100 x i32] zeroinitializer, align 16

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z15enter_exit_datav() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.ALWAYS.TO:AGGRHEAD"([100 x i32]* @a, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 7), i64 68) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  call void @_Z22modify_array_on_targetv()
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(), "QUAL.OMP.MAP.ALWAYS.FROM:AGGRHEAD"([100 x i32]* @a, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 9), i64 52) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z22modify_array_on_targetv() #2

attributes #0 = { noinline optnone uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 1, !"a", i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
