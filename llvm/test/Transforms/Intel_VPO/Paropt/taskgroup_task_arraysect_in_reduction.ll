; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int a[5] = {2, 2, 2, 2, 2};
; int *b;
;
; void foo() {
; // #pragma omp parallel
; // #pragma omp master
; #pragma omp taskgroup task_reduction(*:b[1:4])
;   {
; #pragma omp task in_reduction(*:b[1:4])
;     {
; //      printf("  %d %d %d %d\n", b[1], b[2], b[3], b[4]);
;       b[1] *= 2; b[4] *= 3 ;
; //      printf("  %d %d %d %d\n",  b[1], b[2], b[3], b[4]);
;     }
;   }
; }
;
; // int main() {
; //   b = &a[0];
; //   foo();
; //   printf("%d %d %d %d %d\n", a[0], a[1], a[2], a[3], a[4]);
; //   return 0;
; // }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

; Check that the shared struct has space for @b, and
; reduction item struct type is defined.
; CHECK: %__struct.kmp_privates.t = type {}
; CHECK: %__struct.kmp_taskred_input_t = type { ptr, ptr, i64, ptr, ptr, ptr, i32 }
; CHECK: %__struct.shared.t = type { ptr }

; Check that the reduction struct is correctly initialized
; using the size of i16, and a dereference of the byref %a.addr
; CHECK: [[RED_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[B_LOAD:%[^ ]+]] = load ptr, ptr @b
; CHECK: [[B_LOAD_PLUS_OFFSET:%[^ ]+]] = getelementptr i32, ptr [[B_LOAD]], i64 1
; CHECK: store ptr [[B_LOAD_PLUS_OFFSET]], ptr [[RED_ITEM_PTR_GEP]]
; CHECK: [[ORIG_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 1
; CHECK: store ptr [[B_LOAD_PLUS_OFFSET]], ptr [[ORIG_ITEM_PTR_GEP]]
; CHECK: [[RED_ITEM_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %{{[^ ,]+}}, i32 0, i32 2
; CHECK: store i64 16, ptr [[RED_ITEM_SIZE_GEP]]

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[B_RED:%[^ ]+]] = call ptr @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: [[B_RED_MINUS_OFFSET:%[^ ]+]] = getelementptr i32, ptr [[B_RED]], i64 -1
; CHECK: store ptr [[B_RED_MINUS_OFFSET]], ptr {{%[^ ]+}}

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @{{.*}}task_red_init{{.*}}(ptr [[PRIV_ARG:%[^ ]+]], ptr [[ORIG_ARG:%[^ ]+]]) {
; Check that the ending GEP for the array to be initialized is being computed
; CHECK: {{%[^ ]+}} = getelementptr i32, ptr [[PRIV_ARG]], i64 4
; CHECK: store i32 1, ptr {{.*}}

; CHECK: define internal void @{{.*}}task_red_comb{{.*}}(ptr [[RHS:%[^ ]+]], ptr [[LHS:%[^ ]+]]) {
; Check that the ending GEP for the array to be initialized is being computed
; CHECK: {{%[^ ]+}} = getelementptr i32, ptr [[RHS]], i64 4
; CHECK: [[LHS_VAL:%[^ ]]] = load i32, ptr {{.*}}
; CHECK: [[RHS_VAL:%[^ ]]] = load i32, ptr {{.*}}
; CHECK: [[RES:%[^ ]+]] = mul i32 [[RHS_VAL]], [[LHS_VAL]]
; CHECK: store i32 [[RES]], ptr {{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [5 x i32] [i32 2, i32 2, i32 2, i32 2, i32 2], align 16
@b = dso_local global ptr null, align 8

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.REDUCTION.MUL:ARRSECT.PTR_TO_PTR.TYPED"(ptr @b, i32 0, i64 4, i64 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.INREDUCTION.MUL:ARRSECT.PTR_TO_PTR.TYPED"(ptr @b, i32 0, i64 4, i64 1) ]

  %2 = load ptr, ptr @b, align 8
  %arrayidx = getelementptr inbounds i32, ptr %2, i64 1
  %3 = load i32, ptr %arrayidx, align 4
  %mul = mul nsw i32 %3, 2
  store i32 %mul, ptr %arrayidx, align 4
  %4 = load ptr, ptr @b, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %4, i64 4
  %5 = load i32, ptr %arrayidx1, align 4
  %mul2 = mul nsw i32 %5, 3
  store i32 %mul2, ptr %arrayidx1, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
