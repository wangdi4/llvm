; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; int a[100];
; #pragma omp end declare target
;
; extern void modify_array_on_target();
;
; void update() {
;   modify_array_on_target();
; #pragma omp target update from(a)
; }

; CHECK-DAG: @[[SIZE1:.+]] = {{.*}} constant {{.*}} [i64 400]
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 2]
; CHECK-DAG: %[[A1:.+]] = bitcast i8*{{.*}}@a
; CHECK-DAG: %[[CAST1_1:.+]] = bitcast {{.*}} %[[A1]] to i8*
; CHECK-DAG: store i8* %[[CAST1_1]]
; CHECK-DAG: %[[CAST1_2:.+]] = bitcast {{.*}} %[[A1]] to i8*
; CHECK-DAG: store i8* %[[CAST1_2]]
; Verify that the descriptor for 'a' is passed to __tgt_target_data_update
; CHECK-DAG: call void @__tgt_target_data_update(i64 %{{.*}}, i32 1, {{.*}}@[[SIZE1]]{{.*}}@[[MAPTYPE1]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local target_declare global [100 x i32] zeroinitializer, align 16

define dso_local void @_Z6updatev() {
entry:
  call void @_Z22modify_array_on_targetv()
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.MAP.FROM"([100 x i32]* @a, [100 x i32]* @a, i64 400, i64 2, i8* null, i8* null) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  ret void
}

declare dso_local void @_Z22modify_array_on_targetv()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 1, !"a", i32 0, i32 0, [100 x i32]* @a}
