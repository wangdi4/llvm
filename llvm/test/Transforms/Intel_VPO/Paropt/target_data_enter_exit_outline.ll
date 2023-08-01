; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

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
; CHECK-DAG: @[[MAPTYPE1:.+]] = {{.*}} constant {{.*}} [i64 6]
; CHECK-DAG: @[[SIZE2:.+]] = {{.*}} constant {{.*}} [i64 68]
; CHECK-DAG: @[[MAPTYPE2:.+]] = {{.*}} constant {{.*}} [i64 5]
; CHECK-DAG: store ptr @a, ptr %{{.*}}
; CHECK-DAG: store ptr getelementptr inbounds ([100 x i32], ptr @a, i64 0, i64 7), ptr %{{.*}}

; Verify that the descriptor for 'a' is passed to __tgt_target_data_begin
; CHECK-DAG: call void @__tgt_target_data_begin(i64 %{{.*}}, i32 1, ptr {{.*}}, ptr {{.*}}, ptr @[[SIZE2]], ptr @[[MAPTYPE2]])

; Verify that the descriptor for 'a' is passed to __tgt_target_data_exit
; CHECK-DAG: store ptr @a, ptr %{{.*}}
; CHECK-DAG: store ptr getelementptr inbounds ([100 x i32], ptr @a, i64 0, i64 9), ptr %{{.*}}
; CHECK-DAG: call void @__tgt_target_data_end(i64 %{{.*}}, i32 1, ptr {{.*}}, ptr {{.*}}, ptr @[[SIZE1]], ptr @[[MAPTYPE1]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local target_declare global [100 x i32] zeroinitializer, align 16

define dso_local void @_Z15enter_exit_datav() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.MAP.ALWAYS.TO"(ptr @a, ptr getelementptr inbounds ([100 x i32], ptr @a, i64 0, i64 7), i64 68, i32 5, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]

  call void @_Z22modify_array_on_targetv()

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.MAP.ALWAYS.FROM"(ptr @a, ptr getelementptr inbounds ([100 x i32], ptr @a, i64 0, i64 9), i64 52, i32 6, ptr null, ptr null) ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @_Z22modify_array_on_targetv()

!omp_offload.info = !{!0}
!0 = !{i32 1, !"a", i32 0, i32 0, ptr @a}
